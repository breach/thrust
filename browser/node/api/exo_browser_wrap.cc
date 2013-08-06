// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/exo_browser_wrap.h"

#include "content/public/browser/browser_thread.h"
#include "breach/browser/ui/exo_browser.h"
#include "breach/browser/ui/exo_frame.h"
#include "breach/browser/node/api/exo_frame_wrap.h"

namespace breach {

Persistent<Function> ExoBrowserWrap::s_constructor;

void 
ExoBrowserWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("_ExoBrowser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  /* Prototype */
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_kill"),
      FunctionTemplate::New(Kill)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_size"),
      FunctionTemplate::New(Size)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_position"),
      FunctionTemplate::New(Position)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_addFrame"),
      FunctionTemplate::New(AddFrame)->GetFunction());

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewSymbol("_createNewExoBrowser"),
      FunctionTemplate::New(CreateNewExoBrowser)->GetFunction());
}


ExoBrowserWrap::ExoBrowserWrap()
{
}

ExoBrowserWrap::~ExoBrowserWrap()
{
  LOG(INFO) << "ExoBrowserWrap Destructor";

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::DeleteTask, browser_));
}

void 
ExoBrowserWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  ExoBrowserWrap* browser_w = new ExoBrowserWrap();
  browser_w->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void 
ExoBrowserWrap::CreateNewExoBrowser(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> browser_o = c->NewInstance();
  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(browser_o);

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::CreateTask, browser_w),
      base::Bind(&ExoBrowserWrap::CreateCallback, browser_w, cb_p));
}


void
ExoBrowserWrap::CreateTask()
{
  browser_ = ExoBrowser::CreateNew(this, gfx::Size());
}

void 
ExoBrowserWrap::CreateCallback(
    Persistent<Function>* cb_p)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> browser_o = Local<Object>::New(Isolate::GetCurrent(), 
                                               this->persistent());

  Local<v8::Value> argv[1] = { browser_o };
  cb->Call(browser_o, 1, argv);

  cb_p->Dispose();
  delete cb_p;
}

void
ExoBrowserWrap::DeleteTask(
    ExoBrowser* browser)
{
  LOG(INFO) << "ExoBrowserWrap DeleteTask";
  if(!browser->IsKilled())
    browser->Kill();
  delete browser;
}


/******************************************************************************/
/*                       WRAPPERS, TASKS & CALLBACKS                          */
/******************************************************************************/

void
ExoBrowserWrap::Kill(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::KillTask, browser_w),
      base::Bind(&ExoBrowserWrap::KillCallback, browser_w, cb_p));
}


void
ExoBrowserWrap::KillTask()
{
  if(!browser_->is_killed())
    browser_->Kill();
}

void
ExoBrowserWrap::KillCallback(
    Persistent<Function>* cb_p)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> browser_o = Local<Object>::New(Isolate::GetCurrent(), 
                                               this->persistent());

  cb->Call(browser_o, 0, NULL);

  cb_p->Dispose();
  delete cb_p;
}


void
ExoBrowserWrap::Size(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  gfx::Size* size = new gfx::Size();

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::SizeTask, browser_w, size),
      base::Bind(&ExoBrowserWrap::SizeCallback, browser_w, cb_p, size));
}

void
ExoBrowserWrap::SizeTask(
    gfx::Size *size)
{
  if(!browser_->is_killed())
    (*size) = browser_->size();
}

void
ExoBrowserWrap::SizeCallback(
    Persistent<Function>* cb_p,
    gfx::Size *size)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> browser_o = Local<Object>::New(Isolate::GetCurrent(), 
                                               this->persistent());

  Local<Array> size_array = Array::New();
  size_array->Set(0, Integer::New(size->width()));
  size_array->Set(1, Integer::New(size->height()));

  Local<v8::Value> argv[1] = { size_array };
  cb->Call(browser_o, 1, argv);

  cb_p->Dispose();
  delete cb_p;
  delete size;
}


void
ExoBrowserWrap::Position(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  gfx::Point* position = new gfx::Point();

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::PositionTask, browser_w, position),
      base::Bind(&ExoBrowserWrap::PositionCallback, browser_w, cb_p, position));
}

ExoBrowserWrap::PositionTask(
    gfx::Point *position)
{
  if(!browser_->is_killed())
    (*position) = browser_->position();
}

void
ExoBrowserWrap::PositionCallback(
    Persistent<Function>* cb_p,
    gfx::Point *position)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> browser_o = Local<Object>::New(Isolate::GetCurrent(), 
                                               this->persistent());

  Local<Array> position_array = Array::New();
  position_array->Set(0, Integer::New(position->x()));
  position_array->Set(1, Integer::New(position->y()));

  Local<v8::Value> argv[1] = { position_array };
  cb->Call(wrap, 1, argv);

  cb_p->Dispose();
  delete cb_p;
  delete position;
}


void 
ExoBrowserWrap::AddFrame(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: frame */
  Local<Object> frame_o = Local<Object>::Cast(args[0]);
  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::AddFrameTask, browser_w, frame_w),
      base::Bind(&ExoBrowserWrap::AddFrameCallback, browser_w, cb_p));
}


void
ExoBrowserWrap::AddFrameTask(
    ExoFrameWrap* frame_w)
{
  if(!browser_->is_killed()) {
    browser_->addFrame(frame_w->frame_)
  }
}


void
ExoBrowserWrap::AddFrameCallback(
    Persistent<Function>* cb_p)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> browser_o = Local<Object>::New(Isolate::GetCurrent(), 
                                               this->persistent());

  cb->Call(browser_o, 0, NULL);

  cb_p->Dispose();
  delete cb_p;
}



} // namespace breach
    
