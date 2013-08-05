// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/exo_browser_wrap.h"

#include "content/public/browser/browser_thread.h"
#include "breach/browser/ui/browser.h"
#include "breach/browser/breach_content_browser_client.h"
#include "breach/browser/breach_browser_context.h"

Persistent<Function> BrowserWrap::s_constructor;

void 
ExoBrowserWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("ExoBrowser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("kill"),
      FunctionTemplate::New(Kill)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("size"),
      FunctionTemplate::New(Size)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("position"),
      FunctionTemplate::New(Position)->GetFunction());

  /*
  tpl->PrototypeTemplate()->Set(String::NewSymbol("addFrame"),
      FunctionTemplate::New(AddFrame)->GetFunction());
  */

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewSymbol("createNewExoBrowser"),
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
      base::Bind(&BrowserWrap::DeleteTask, browser_));
}


void 
ExoBrowserWrap::CreateNewExoBrowser(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> wrap = c->NewInstance();
  BrowserWrap* obj = ObjectWrap::Unwrap<BrowserWrap>(wrap);

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *pcb = new Persistent<Function>();
  pcb->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&BrowserWrap::CreateTask, obj),
      base::Bind(&BrowserWrap::CreateCallback, obj, pcb));
}


void 
ExoBrowserWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  ExoBrowserWrap* wrap = new ExoBrowserWrap();
  wrap->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
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

ExoBrowserWrap::CreateTask()
{
  browser_ = ExoBrowser::CreateNew(this, gfx::Size());
}

void 
ExoBrowserWrap::CreateCallback(
    Persistent<Function>* pcb)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *pcb);
  Local<Object> wrap = Local<Object>::New(Isolate::GetCurrent(), 
                                          this->persistent());

  Local<v8::Value> argv[1] = { wrap };
  cb->Call(wrap, 1, argv);

  pcb->Dispose();
  delete pcb;
}


/******************************************************************************/
/*                       WRAPPERS, TASKS & CALLBACKS                          */
/******************************************************************************/

void
ExoBrowserWrap::Kill(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *pcb = new Persistent<Function>();
  pcb->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* obj = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::KillTask, obj),
      base::Bind(&ExoBrowserWrap::KillCallback, obj, pcb));
}


void
BrowserWrap::KillTask()
{
  if(!browser_->is_killed())
    browser_->Kill();
}

void
BrowserWrap::KillCallback(
    Persistent<Function>* pcb)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *pcb);
  Local<Object> wrap = Local<Object>::New(Isolate::GetCurrent(), 
                                          this->persistent());

  cb->Call(wrap, 0, NULL);

  pcb->Dispose();
  delete pcb;
}


void
ExoBrowserWrap::Size(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *pcb = new Persistent<Function>();
  pcb->Reset(Isolate::GetCurrent(), cb);

  gfx::Size* size = new gfx::Size();

  ExoBrowserWrap* obj = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::SizeTask, obj, size),
      base::Bind(&ExoBrowserWrap::SizeCallback, obj, pcb, size));
}

void
BrowserWrap::SizeTask(
    gfx::Size *size)
{
  if(!browser_->is_killed())
    (*size) = browser_->size();
}

void
BrowserWrap::SizeCallback(
    Persistent<Function>* pcb,
    gfx::Size *size)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *pcb);
  Local<Object> wrap = Local<Object>::New(Isolate::GetCurrent(), 
                                          this->persistent());

  Local<Array> size_array = Array::New();
  size_array->Set(0, Integer::New(size->width()));
  size_array->Set(1, Integer::New(size->height()));

  Local<v8::Value> argv[1] = { size_array };
  cb->Call(wrap, 1, argv);

  pcb->Dispose();
  delete pcb;
  delete size;
}


void
ExoBrowserWrap::Position(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *pcb = new Persistent<Function>();
  pcb->Reset(Isolate::GetCurrent(), cb);

  gfx::Point* position = new gfx::Point();

  ExoBrowserWrap* obj = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::PositionTask, obj, position),
      base::Bind(&ExoBrowserWrap::PositionCallback, obj, pcb, position));
}

BrowserWrap::PositionTask(
    gfx::Point *position)
{
  if(!browser_->is_killed())
    (*position) = browser_->position();
}

void
BrowserWrap::PositionCallback(
    Persistent<Function>* pcb,
    gfx::Point *position)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *pcb);
  Local<Object> wrap = Local<Object>::New(Isolate::GetCurrent(), 
                                          this->persistent());

  Local<Array> position_array = Array::New();
  position_array->Set(0, Integer::New(position->x()));
  position_array->Set(1, Integer::New(position->y()));

  Local<v8::Value> argv[1] = { position_array };
  cb->Call(wrap, 1, argv);

  pcb->Dispose();
  delete pcb;
  delete position;
}





} // namespace breach
    
