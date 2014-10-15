// Copyright (c) 2014 Stanislas Polu. All rights reserved.
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

using namespace content;

namespace thrust_shell {

const char kSocketBoundary[] = "--(Foo)++__THRUST_SHELL_BOUNDARY__++(Bar)--";
const char kAPIServerThreadName[] = "thrust_shell_api_server_thread";

/******************************************************************************/
/* APISERVER::CLIENT::REMOTE */
/******************************************************************************/
APIServer::Client::Remote::Remote(
    APIServer::Client* client,
    unsigned int target)
  : client_(client),
    target_(target)
{
}

void 
APIServer::Client::Remote::InvokeMethod(
    const std::string method,
    scoped_ptr<base::DictionaryValue> args,
    const API::MethodCallback& callback)
{
  /* Runs on UI Thread. */
  LOG(INFO) << "Remote::Client::InvokeMethod [" << target_ << "] " << this;
  
  client_->server_->thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&APIServer::Client::SendInvoke, client_, 
                 callback, target_, method, base::Passed(args.Pass())));
}

void APIServer::Client::Remote::EmitEvent(
    const std::string type,
    scoped_ptr<base::DictionaryValue> event)
{
  /* Runs on UI Thread. */
  LOG(INFO) << "Remote::Client::EmitEvent [" << target_ << "] " << this;

  client_->server_->thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&APIServer::Client::SendEvent, client_, 
                 target_, type, base::Passed(event.Pass())));
}

/******************************************************************************/
/* APISERVER::CLIENT */
/******************************************************************************/
APIServer::Client::Client(
    APIServer* server, 
    API* api,
    scoped_ptr<net::StreamListenSocket> conn)
  : server_(server),
    api_(api),
    action_id_(0)
{
  conn_ = conn.Pass();
}

APIServer::Client::~Client() 
{
  LOG(INFO) << "APIServer::Client Destructor: " << this;
  conn_.reset();

  /* We start by deleting all bindings that are not sessions. */
  std::map<unsigned int, scoped_refptr<Remote> >::iterator it = remotes_.begin();
  while(it != remotes_.end()) {
    if(api_->GetBinding(it->first)->type() != "session") {
      /* Remotes will be removed from the API with the Delete call */
      api_->Delete(it->first);
      remotes_.erase(it++);
    }
    else {
      ++it;
    }
  }
  /* Then we delete the sessions once nothing is left depending on them. */
  it = remotes_.begin();
  while(it != remotes_.end()) {
    /* Remotes will be removed from the API with the Delete call */
    api_->Delete(it->first);
    ++it;
  }

  remotes_.clear();
}

void
APIServer::Client::ProcessChunk(
    std::string chunk)
{
  acc_ += chunk;
  /* Runs on APIServer Thread. */
  size_t pos;
  while((pos = acc_.find(kSocketBoundary)) != std::string::npos) {
    std::string raw = acc_.substr(0, pos);
    acc_ = acc_.substr(pos + strlen(kSocketBoundary));
    if(raw.length() == 0) {
      continue;
    }

    scoped_ptr<base::DictionaryValue> action;
    action.reset(
        static_cast<base::DictionaryValue*>(base::JSONReader::Read(raw)));

    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&APIServer::Client::PerformAction, this, 
          base::Passed(action.Pass())));
  }
}

void
APIServer::Client::ReplyToAction(
    const unsigned int id,
    const std::string& error, 
    scoped_ptr<base::DictionaryValue> result)
{
  /* Runs on UI Thread. */
  server_->thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&APIServer::Client::SendReply, this, 
                 id, error, base::Passed(result.Pass())));
}

void
APIServer::Client::PerformAction(
    scoped_ptr<base::DictionaryValue> _action)
{
  /* Runs on UI Thread. */

  /* We extract the relevant fields form the actio object. */
  std::string action;
  _action->GetString("_action", &action);

  int id = 0;
  _action->GetInteger("_id", &id);

  int target = 0;
  _action->GetInteger("_target", &target);

  std::string method = "";
  _action->GetString("_method", &method);

  std::string type = "";
  _action->GetString("_type", &type);

  scoped_ptr<base::DictionaryValue> args;
  base::DictionaryValue* args_d;
  if(_action->GetDictionary("_args", &args_d)) {
    args.reset(args_d->DeepCopyWithoutEmptyChildren());
  }

  scoped_ptr<base::DictionaryValue> event;
  base::DictionaryValue* event_d;
  if(_action->GetDictionary("_event", &event_d)) {
    event.reset(event_d->DeepCopyWithoutEmptyChildren());
  }

  scoped_ptr<base::DictionaryValue> result;
  base::DictionaryValue* result_d;
  if(_action->GetDictionary("_result", &result_d)) {
    result.reset(result_d->DeepCopyWithoutEmptyChildren());
  }

  std::string error = "";
  _action->GetString("_error", &error);

  /*
  LOG(INFO) << "===========================================";
  LOG(INFO) << "action: " << action;
  LOG(INFO) << "id: " << id;
  LOG(INFO) << "target: " << target;
  LOG(INFO) << "method: " << method;
  LOG(INFO) << "type: " << type;
  LOG(INFO) << "args: " << args.get();
  LOG(INFO) << "event: " << event.get();
  LOG(INFO) << "result: " << result.get();
  LOG(INFO) << "error: " << error;
  LOG(INFO) << "===========================================";
  */

  if(action.compare("create") == 0 && type.length()) {
    unsigned int target = api_->Create(type, args.Pass());
    remotes_[target] = new Remote(this, target);
    api_->SetRemote(target, remotes_[target].get());

    base::DictionaryValue* res = new base::DictionaryValue;
    res->SetInteger("_target", target);
    ReplyToAction(id, std::string(""), 
                  scoped_ptr<base::DictionaryValue>(res).Pass());
  }
  else if(action.compare("call") == 0 && target > 0) {
    api_->CallMethod(target, method, args.Pass(),
                     base::Bind(&APIServer::Client::ReplyToAction, this, id));
  }
  else if(action.compare("delete") == 0 && target > 0) {
    api_->Delete(target);
    remotes_.erase(target);

    base::DictionaryValue* res = new base::DictionaryValue;
    ReplyToAction(id, std::string(""),
                  scoped_ptr<base::DictionaryValue>(res).Pass());
  }
  else if(action.compare("reply") == 0 && 
          invokes_.find(id) != invokes_.end()) {
    invokes_[id].Run(error, result.Pass());
    invokes_.erase(id);
  }
  else {
    LOG(INFO) << "[API_SERVER] IGNORED: " << action << " " << id;
  }
}



void
APIServer::Client::SendReply(
  const unsigned int id,
  const std::string& error, 
  scoped_ptr<base::DictionaryValue> result)
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
APIServer::Client::SendEvent(
    unsigned int target,
    const std::string type,
    scoped_ptr<base::DictionaryValue> event)
{
  /* Runs on APIServer Thread. */
  base::DictionaryValue action;
  action.SetString("_action", "event");
  action.SetInteger("_id", ++action_id_);
  action.SetInteger("_target", target);
  action.SetString("_type", type);
  action.Set("_event", event->DeepCopy());

  std::string payload;
  base::JSONWriter::Write(&action, &payload);
  payload += "\n" + std::string(kSocketBoundary) + "\n";

  if(conn_) {
    conn_->Send(payload);
  }
}

void 
APIServer::Client::SendInvoke(
    const API::MethodCallback& callback,
    unsigned int target,
    const std::string method,
    scoped_ptr<base::DictionaryValue> args)
{
  /* Runs on APIServer Thread. */
  unsigned int action_id = ++action_id_;

  invokes_[action_id] = callback;

  base::DictionaryValue action;
  action.SetString("_action", "invoke");
  action.SetInteger("_id", action_id);
  action.SetInteger("_target", target);
  action.SetString("_method", method);
  action.Set("_args", args->DeepCopy());

  std::string payload;
  base::JSONWriter::Write(&action, &payload);
  payload += "\n" + std::string(kSocketBoundary) + "\n";

  if(conn_) {
    conn_->Send(payload);
  }
}


/******************************************************************************/
/* APISERVER */
/******************************************************************************/
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
  clients_[connection.get()] = new Client(this, api_, connection.Pass());
}

void 
APIServer::DidRead(
    net::StreamListenSocket* connection,
    const char* data,
    int len)
{
  //LOG(INFO) << "DATA: " << data;
  if(clients_[connection]) {
    clients_[connection]->ProcessChunk(std::string(data, len));
  }
}

void 
APIServer::DidClose(
    net::StreamListenSocket* connection)
{
  LOG(INFO) << "Close " << connection;
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&APIServer::DestroyClient, this, connection));
}

void
APIServer::DestroyClient(
    net::StreamListenSocket* connection)
{
  if(clients_[connection]) {
    clients_.erase(connection);
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

  
} // namespace thrust_shell
