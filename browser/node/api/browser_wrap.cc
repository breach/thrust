// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/browser_wrap.h"

#include "base/command_line.h"
#include "net/base/net_util.h"
#include "content/public/browser/browser_thread.h"
#include "breach/browser/ui/browser.h"
#include "breach/browser/breach_content_browser_client.h"
#include "breach/browser/breach_browser_context.h"


/* TODO(spolu): There is a conflict on `Value`. Find it. Fix it. */
using namespace v8;
using content::BrowserThread;

namespace breach {

/* TODO(spolu) Code check */
static GURL GetStartupURL() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  const CommandLine::StringVector& args = command_line->GetArgs();

  if (args.empty())
    return GURL("http://www.google.com/");

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme())
    return url;

  return net::FilePathToFileURL(base::FilePath(args[0]));
}

Persistent<Function> BrowserWrap::constructor;

BrowserWrap::BrowserWrap()
{
}

BrowserWrap::~BrowserWrap()
{
  LOG(INFO) << "BrowserWrap Destructor";

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&BrowserWrap::DeleteTask, browser_));
}

void 
BrowserWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Browser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("close"),
      FunctionTemplate::New(Close)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("showDevTools"),
      FunctionTemplate::New(ShowDevTools)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("closeDevTools"),
      FunctionTemplate::New(CloseDevTools)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("size"),
      FunctionTemplate::New(Size)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("position"),
      FunctionTemplate::New(Position)->GetFunction());

  constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());
  //exports->Set(String::NewSymbol("Browser"), tpl->GetFunction());

  exports->Set(String::NewSymbol("createNewBrowser"),
      FunctionTemplate::New(CreateNewBrowser)->GetFunction());
}

void 
BrowserWrap::CreateNewBrowser(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> c = Local<Function>::New(Isolate::GetCurrent(), constructor);
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
BrowserWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  BrowserWrap* browser_wrap = new BrowserWrap();
  browser_wrap->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void
BrowserWrap::Close(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  BrowserWrap* obj = ObjectWrap::Unwrap<BrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&BrowserWrap::CloseTask, obj));
}

void
BrowserWrap::ShowDevTools(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  BrowserWrap* obj = ObjectWrap::Unwrap<BrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&BrowserWrap::ShowDevToolsTask, obj));
}

void
BrowserWrap::CloseDevTools(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  BrowserWrap* obj = ObjectWrap::Unwrap<BrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&BrowserWrap::CloseDevToolsTask, obj));
}

void
BrowserWrap::Size(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  BrowserWrap* obj = ObjectWrap::Unwrap<BrowserWrap>(args.This());
  gfx::Size s = obj->SizeGetter();

  Local<Array> size_array = Array::New();
  size_array->Set(0, Integer::New(s.width()));
  size_array->Set(1, Integer::New(s.height()));

  args.GetReturnValue().Set(size_array);
}


void
BrowserWrap::Position(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  BrowserWrap* obj = ObjectWrap::Unwrap<BrowserWrap>(args.This());
  gfx::Point p = obj->PositionGetter();

  Local<Array> position_array = Array::New();
  position_array->Set(0, Integer::New(p.x()));
  position_array->Set(1, Integer::New(p.y()));

  args.GetReturnValue().Set(position_array);
}


/******************************************************************************/
/*                                  TASKS                                     */
/******************************************************************************/
void
BrowserWrap::DeleteTask(
    Browser* browser)
{
  if(!browser->IsClosed())
    /* will cause browser deletion */
    browser->Close();
}

void
BrowserWrap::CreateTask()
{
  browser_ = Browser::CreateNewWindow(
                   BreachContentBrowserClient::Get()->browser_context(),
                   GetStartupURL(),
                   NULL,
                   MSG_ROUTING_NONE,
                   gfx::Size());
}

void 
BrowserWrap::CreateCallback(
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

void
BrowserWrap::CloseTask()
{
  if(!browser_->IsClosed()) {
    browser_->Close();
  }
}

void
BrowserWrap::ShowDevToolsTask()
{
  if(!browser_->IsClosed())
    browser_->ShowDevTools();
}

void
BrowserWrap::CloseDevToolsTask()
{
  if(!browser_->IsClosed())
    browser_->CloseDevTools();
}


/******************************************************************************/
/*                                 GETTERS                                    */
/******************************************************************************/
gfx::Size
BrowserWrap::SizeGetter()
{
  if(!browser_->IsClosed()) {
    return browser_->Size();
  }
  else return gfx::Size();
}

gfx::Point
BrowserWrap::PositionGetter()
{
  if(!browser_->IsClosed()) {
    return browser_->Position();
  }
  else return gfx::Point();
}

    
} // namespace breach

