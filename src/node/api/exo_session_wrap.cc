// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "exo_browser/src/node/api/exo_session_wrap.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"

#include "exo_browser/src/browser/session/exo_session.h"
#include "exo_browser/src/browser/ui/exo_browser.h"
#include "exo_browser/src/browser/ui/exo_frame.h"
#include "exo_browser/src/node/node_thread.h"

using namespace v8;

namespace exo_browser {

Persistent<Function> ExoSessionWrap::s_constructor;

void 
ExoSessionWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("_ExoSession"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  /* Prototype */
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_off_the_record"),
      FunctionTemplate::New(OffTheRecord)->GetFunction());

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewSymbol("_createExoSession"),
      FunctionTemplate::New(CreateExoSession)->GetFunction());
}

ExoSessionWrap::ExoSessionWrap()
{
}

ExoSessionWrap::~ExoSessionWrap()
{
  LOG(INFO) << "ExoSessionWrap Destructor";

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::DeleteTask, session_));
}

void 
ExoSessionWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  ExoSessionWrap* session_w = new ExoSessionWrap();
  session_w->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void 
ExoSessionWrap::CreateExoSession(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> session_o = c->NewInstance();

  /* We keep a Peristent as the object will be returned asynchronously. */
  Persistent<Object> *session_p = new Persistent<Object>();
  session_p->Reset(Isolate::GetCurrent(), session_o);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(session_o);

  /* args[0]: spec = { off_the_record, path } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  bool off_the_record = 
    spec->Get(String::New("off_the_record"))->ToBoolean()->Value();
  std::string path = std::string(
      *String::Utf8Value(Local<String>::Cast(spec->Get(String::New("path")))));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::CreateTask, session_w, 
                 off_the_record, path, session_p, cb_p));
}

void
ExoSessionWrap::CreateTask(
    const bool off_the_record,
    const std::string& path,
    Persistent<Object>* session_p,
    Persistent<Function>* cb_p)
{
  session_ = new ExoSession(off_the_record,
                            path,
                            this);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoSessionWrap::CreateCallback, this, session_p, cb_p));
}

void
ExoSessionWrap::CreateCallback(
    Persistent<Object>* session_p,
    Persistent<Function>* cb_p)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> session_o = Local<Object>::New(Isolate::GetCurrent(),
                                               *session_p);

  LOG(INFO) << "CreateCallback ExoSessionWrap";
  Local<v8::Value> argv[1] = { session_o };
  cb->Call(session_o, 1, argv);

  cb_p->Dispose();
  delete cb_p;
  session_p->Dispose();
  delete session_p;
}

void
ExoSessionWrap::DeleteTask(
    ExoSession* session)
{
  LOG(INFO) << "ExoSessionWrap DeleteTask";
  delete session;
}


/******************************************************************************/
/*                              WRAPPERS, TASKS                               */
/******************************************************************************/

void 
ExoSessionWrap::OffTheRecord(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  bool* off_the_record = new bool;

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::OffTheRecordTask, 
                 session_w, off_the_record, cb_p));
}


void
ExoSessionWrap::OffTheRecordTask(
    bool* off_the_record,
    Persistent<Function>* cb_p)
{
  if(session_ != NULL)
    (*off_the_record) = session_->IsOffTheRecord();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoSessionWrap::BooleanCallback, this, cb_p, off_the_record));
}


} // namespace exo_browser

