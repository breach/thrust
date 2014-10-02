// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/api/api_server.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "net/socket/socket_descriptor.h"
#include "net/socket/unix_domain_socket_posix.h"
#include "content/public/browser/browser_thread.h"

#include "src/api/api_binding.h"

using namespace content;

namespace exo_shell {

const char kSocketBoundary[] = "--(Foo)++__EXO_SHELL_BOUNDARY__++(Bar)--";
const char kAPIServerThreadName[] = "exo_shell_api_server_thread";

APIServer::APIServer(
    API* api,
    const base::FilePath& socket_path)
  : api_(api),
    socket_path_(socket_path)
{
}

void 
APIServer::Start()
{
  if(thread_)
    return;
  thread_.reset(new base::Thread(kAPIServerThreadName));

  BrowserThread::PostTask(
      BrowserThread::FILE, FROM_HERE,
      base::Bind(&APIServer::StartHandlerThread, this));
}

void 
APIServer::Stop() {
  if (!thread_)
    return;
  BrowserThread::PostTaskAndReply(
      BrowserThread::FILE, FROM_HERE,
      base::Bind(&APIServer::StopHandlerThread, this),
      base::Bind(&APIServer::ResetHandlerThread, this));
}

void 
APIServer::DidAccept(
    net::StreamListenSocket* server,                          
    scoped_ptr<net::StreamListenSocket> connection)
{
  LOG(INFO) << "Accept";
  conn_ = connection.Pass();
}

void 
APIServer::DidRead(
    net::StreamListenSocket* connection,
    const char* data,
    int len)
{
  LOG(INFO) << "DATA: " << data;
  acc_ += std::string(data, len);
  ProcessData();
}

void 
APIServer::DidClose(
    net::StreamListenSocket* sock)
{
  LOG(INFO) << "Close";
  conn_.reset();
}


void
APIServer::ReplyToAction(
    const unsigned int id,
    const std::string& error, 
    scoped_ptr<base::Value> result)
{
  /* Runs on UI Thread. */
  thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&APIServer::SendReply, this, 
                 id, error, base::Passed(result.Pass())));
}

void
APIServer::PerformAction(
    std::string action,
    unsigned int id,
    scoped_ptr<base::DictionaryValue> args,
    unsigned int target,
    std::string type,
    std::string method)
{
  /* Runs on UI Thread. */
  LOG(INFO) << "action: " << action;
  LOG(INFO) << "id: " << id;
  LOG(INFO) << "args: " << args.get();

  LOG(INFO) << "target: " << target;
  LOG(INFO) << "type: " << type;
  LOG(INFO) << "method: " << target;

  if(action.compare("create") == 0 && type.length()) {
    unsigned int target = api_->Create(type, args.Pass());
    base::DictionaryValue* res = new base::DictionaryValue;
    res->SetInteger("_target", target);
    ReplyToAction(id, std::string(""),  
                  scoped_ptr<base::Value>(res).Pass());
  }
  else if(action.compare("call") == 0 && target > 0) {
    api_->CallMethod(target, method, args.Pass(),
                     base::Bind(&APIServer::ReplyToAction, this, id));
  }
  else if(action.compare("delete") == 0 && target > 0) {
    api_->Delete(target);

    scoped_ptr<base::Value> null;
    null.reset(base::Value::CreateNullValue());
    ReplyToAction(id, std::string(""), null.Pass());
  }
  else {
    LOG(INFO) << "[API_SERVER] IGNORED: " << action << " " << id;
  }
}



void
APIServer::SendReply(
  const unsigned int id,
  const std::string& error, 
  scoped_ptr<base::Value> result)
{
  /* Runs on APIServer Thread. */
  base::DictionaryValue action;
  action.SetString("_action", "reply");
  action.SetInteger("_id", id);
  action.SetString("_error", error);
  action.Set("_result", result->DeepCopy());

  std::string payload;
  base::JSONWriter::Write(&action, &payload);
  payload += "\n" + std::string(kSocketBoundary) + "\n";

  if(conn_) {
    conn_->Send(payload);
  }
}

void
APIServer::ProcessData()
{
  /* Runs on APIServer Thread. */
  size_t pos;
  while((pos = acc_.find(kSocketBoundary)) != std::string::npos) {
    std::string raw = acc_.substr(0, pos);
    acc_ = acc_.substr(pos + strlen(kSocketBoundary));
    if(raw.length() == 0) {
      continue;
    }

    scoped_ptr<base::Value> root;
    root.reset(base::JSONReader::Read(raw));

    base::DictionaryValue* dict = static_cast<base::DictionaryValue*>(root.get());

    std::string _action;
    dict->GetString("_action", &_action);
    int _id = 0;
    dict->GetInteger("_id", &_id);

    base::DictionaryValue* args_d;
    dict->GetDictionary("_args", &args_d);
    scoped_ptr<base::DictionaryValue> _args;
    _args.reset(args_d->DeepCopyWithoutEmptyChildren());

    int _target = 0;
    dict->GetInteger("_target", &_target);
    std::string _type;
    dict->GetString("_type", &_type);
    std::string _method;
    dict->GetString("_method", &_method);

    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&APIServer::PerformAction, this, 
                   _action, _id, base::Passed(_args.Pass()), _target, _type, _method));
  }
}



bool 
APIServer::UserCanConnectCallback(
    uid_t user_id, 
    gid_t group_id) {
  return true;
}

void 
APIServer::DeleteSocketFile()
{
  base::DeleteFile(socket_path_, false /* not recursive */);
}



void 
APIServer::StartHandlerThread() 
{
  /* Runs on FILE thread. */
  base::Thread::Options options;
  options.message_loop_type = base::MessageLoop::TYPE_IO;
  if (!thread_->StartWithOptions(options)) {
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&APIServer::ResetHandlerThread, this));
    return;
  }

  thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&APIServer::ThreadInit, this));
}

void 
APIServer::ResetHandlerThread() 
{
  thread_.reset();
}

void 
APIServer::StopHandlerThread() 
{
  /* Runs on FILE thread to make sure that it is serialized against */
  /* {Start|Stop}HandlerThread and to allow calling pthread_join.   */
  if (!thread_->message_loop())
    return;
  thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&APIServer::ThreadTearDown, this));
  // Thread::Stop joins the thread.
  thread_->Stop();
}

void 
APIServer::ThreadInit() 
{
  LOG(INFO) << "Cleaning up: " << socket_path_.value();
  DeleteSocketFile();

  LOG(INFO) << "Listening on: " << socket_path_.value();
  /* Runs on the handler thread */
  socket_ = net::UnixDomainSocket::CreateAndListen(
      socket_path_.value(), this, 
      base::Bind(&APIServer::UserCanConnectCallback, this));
}

void 
APIServer::ThreadTearDown() 
{
  /* Runs on the handler thread */
}

  
} // namespace exo_shell
