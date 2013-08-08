// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/exo_frame_wrap.h"

#include "content/public/browser/browser_thread.h"
#include "breach/browser/ui/exo_browser.h"
#include "breach/browser/ui/exo_frame.h"
#include "breach/browser/node/api/exo_browser_wrap.h"

using namespace v8;

namespace breach {

Persistent<Function> ExoFrameWrap::s_constructor;

void 
ExoFrameWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("_ExoFrame"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  /* Prototype */
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_loadURL"),
      FunctionTemplate::New(LoadURL)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_goBackOrForward"),
      FunctionTemplate::New(GoBackOrForward)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_reload"),
      FunctionTemplate::New(Reload)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_stop"),
      FunctionTemplate::New(Stop)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_name"),
      FunctionTemplate::New(Name)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_type"),
      FunctionTemplate::New(Type)->GetFunction());

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewSymbol("_createExoFrame"),
      FunctionTemplate::New(CreateExoFrame)->GetFunction());
}

ExoFrameWrap::ExoFrameWrap()
{
}

ExoFrameWrap::~ExoFrameWrap()
{
  LOG(INFO) << "ExoFrameWrap Destructor";

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::DeleteTask, frame_));
}

void 
ExoFrameWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  ExoFrameWrap* frame_o = new ExoFrameWrap();
  frame_o->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void 
ExoFrameWrap::CreateExoFrame(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> frame_o = c->NewInstance();
  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

  /* args[0]: spec = { name, url } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  std::string name = std::string(
      *String::Utf8Value(spec->Get(String::New("name"))->ToString()));
  std::string url = std::string(
      *String::Utf8Value(spec->Get(String::New("url"))->ToString()));
  
  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::CreateTask, frame_w, name, url),
      base::Bind(&ExoFrameWrap::CreateCallback, frame_w, cb_p));
}


void
ExoFrameWrap::CreateTask(
    const std::string& name,
    const std::string& url)
{
  frame_ = new ExoFrame(name,
                        this);
  frame_->LoadURL(GURL(url));
}

void
ExoFrameWrap::CreateCallback(
    Persistent<Function>* cb_p)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> frame_o = Local<Object>::New(Isolate::GetCurrent(), 
                                             this->persistent());

  Local<v8::Value> argv[1] = { frame_o };
  cb->Call(frame_o, 1, argv);

  cb_p->Dispose();
  delete cb_p;
}


void
ExoFrameWrap::DeleteTask(
    ExoFrame* frame)
{
  LOG(INFO) << "ExoFrameWrap DeleteTask";
  if(frame->parent() != NULL) {
    DCHECK(!frame->parent()->is_killed());
    frame->parent()->RemoveFrame(frame->name());
  }
  delete frame;
}

/******************************************************************************/
/*                              WRAPPERS, TASKS                               */
/******************************************************************************/

void 
ExoFrameWrap::LoadURL(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: url */
  std::string url = std::string(
      *String::Utf8Value(Local<String>::Cast(args[0])));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());

  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::LoadURLTask, frame_w, url),
      base::Bind(&ExoFrameWrap::EmptyCallback, frame_w, cb_p));
}


void
ExoFrameWrap::LoadURLTask(
    const std::string& url)
{
  frame_->LoadURL(GURL(url));
}


void 
ExoFrameWrap::GoBackOrForward(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: offset */
  int offset = (Local<Integer>::Cast(args[0]))->Value();

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::GoBackOrForwardTask, frame_w, offset),
      base::Bind(&ExoFrameWrap::EmptyCallback, frame_w, cb_p));
}


void
ExoFrameWrap::GoBackOrForwardTask(
    int offset)
{
  frame_->GoBackOrForward(offset);
}


void 
ExoFrameWrap::Reload(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::ReloadTask, frame_w),
      base::Bind(&ExoFrameWrap::EmptyCallback, frame_w, cb_p));
}


void
ExoFrameWrap::ReloadTask()
{
  frame_->Reload();
}

void 
ExoFrameWrap::Stop(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::StopTask, frame_w),
      base::Bind(&ExoFrameWrap::EmptyCallback, frame_w, cb_p));
}


void
ExoFrameWrap::StopTask()
{
  frame_->Stop();
}


void 
ExoFrameWrap::Name(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  std::string* name = new std::string();

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::NameTask, frame_w, name),
      base::Bind(&ExoFrameWrap::StringCallback, frame_w, cb_p, name));
}


void
ExoFrameWrap::NameTask(
    std::string* name)
{
  (*name) = frame_->name();
}


void 
ExoFrameWrap::Type(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  int* type = new int;

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::TypeTask, frame_w, type),
      base::Bind(&ExoFrameWrap::IntCallback, frame_w, cb_p, type));
}


void
ExoFrameWrap::TypeTask(
    int* type)
{
  (*type) = (int)frame_->type();
}

/******************************************************************************/
/*                                DISPATCHERS                                 */
/******************************************************************************/

void
ExoFrameWrap::SetTitleUpdatedCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  frame_w->title_updated_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoFrameWrap::DispatchTitleUpdated(
    const std::string& title)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!title_updated_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), title_updated_cb_);

    Local<String> title_arg = String::New(title.c_str());

    Local<Value> argv[1] = { title_arg };
    cb->Call(frame_o, 1, argv);
  }
}

} // namespace breach
