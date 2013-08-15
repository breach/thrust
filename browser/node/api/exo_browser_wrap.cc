// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/exo_browser_wrap.h"

#include "content/public/browser/browser_thread.h"
#include "breach/browser/ui/exo_browser.h"
#include "breach/browser/ui/exo_frame.h"
#include "breach/browser/node/api/exo_frame_wrap.h"
#include "breach/browser/node/node_thread.h"
#include "content/public/browser/native_web_keyboard_event.h"

using namespace v8;

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

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_addPage"),
      FunctionTemplate::New(AddPage)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_removePage"),
      FunctionTemplate::New(RemovePage)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_showPage"),
      FunctionTemplate::New(ShowPage)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setControl"),
      FunctionTemplate::New(SetControl)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setControlDimension"),
      FunctionTemplate::New(SetControlDimension)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_unsetControl"),
      FunctionTemplate::New(UnsetControl)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setOpenURLCallback"),
      FunctionTemplate::New(SetOpenURLCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setResizeCallback"),
      FunctionTemplate::New(SetResizeCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setKillCallback"),
      FunctionTemplate::New(SetKillCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setFrameCloseCallback"),
      FunctionTemplate::New(SetFrameCloseCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setFrameCreatedCallback"),
      FunctionTemplate::New(SetFrameCreatedCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setFrameKeyboardCallback"),
      FunctionTemplate::New(SetFrameKeyboardCallback)->GetFunction());

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewSymbol("_createExoBrowser"),
      FunctionTemplate::New(CreateExoBrowser)->GetFunction());
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
ExoBrowserWrap::CreateExoBrowser(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> browser_o = c->NewInstance();

  /* We keep a Peristent as the object will be returned asynchronously. */
  Persistent<Object> *browser_p = new Persistent<Object>();
  browser_p->Reset(Isolate::GetCurrent(), browser_o);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(browser_o);

  /* args[0]: spec = { name, url } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  Local<Array> in = Local<Array>::Cast(spec->Get(String::New("size")));
  gfx::Size size(in->Get(Integer::New(0))->ToNumber()->Value(),
                 in->Get(Integer::New(1))->ToNumber()->Value());

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::CreateTask, browser_w, 
                 size, browser_p, cb_p));
}


void
ExoBrowserWrap::CreateTask(
    const gfx::Size& size,
    Persistent<Object>* browser_p,
    Persistent<Function>* cb_p)
{
  browser_ = ExoBrowser::CreateNew(this, size);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::CreateCallback, this, browser_p, cb_p));
}

void 
ExoBrowserWrap::CreateCallback(
    Persistent<Object>* browser_p,
    Persistent<Function>* cb_p)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> browser_o = Local<Object>::New(Isolate::GetCurrent(),
                                               *browser_p);

  LOG(INFO) << "ExoBrowserWrap CreateCallback";
  Local<v8::Value> argv[1] = { browser_o };
  cb->Call(browser_o, 1, argv);

  cb_p->Dispose();
  delete cb_p;
  browser_p->Dispose();
  delete browser_p;
}

void
ExoBrowserWrap::DeleteTask(
    ExoBrowser* browser)
{
  LOG(INFO) << "ExoBrowserWrap DeleteTask";
  if(!browser->is_killed())
    browser->Kill();
  delete browser;
}


/******************************************************************************/
/*                              WRAPPERS, TASKS                               */
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
      base::Bind(&ExoBrowserWrap::KillTask, browser_w, cb_p));
}


void
ExoBrowserWrap::KillTask(
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed())
    browser_->Kill();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
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
      base::Bind(&ExoBrowserWrap::SizeTask, browser_w, size, cb_p));
}

void
ExoBrowserWrap::SizeTask(
    gfx::Size* size,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed())
    (*size) = browser_->size();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::SizeCallback, this, cb_p, size));
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
      base::Bind(&ExoBrowserWrap::PositionTask, browser_w, position, cb_p));
}

void
ExoBrowserWrap::PositionTask(
    gfx::Point* position,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed())
    (*position) = browser_->position();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::PointCallback, this, cb_p, position));
}




void 
ExoBrowserWrap::SetControl(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: type */
  ExoBrowser::CONTROL_TYPE type = 
    (ExoBrowser::CONTROL_TYPE) (Local<Integer>::Cast(args[0]))->Value();

  /* args[1]: frame */
  Local<Object> frame_o = Local<Object>::Cast(args[1]);
  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

  /* args[2]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[2]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      /* TODO(spolu): Fix usage of (void*) */
      base::Bind(&ExoBrowserWrap::SetControlTask, browser_w, 
                 type, (void*)frame_w, cb_p));
}


void
ExoBrowserWrap::SetControlTask(
    ExoBrowser::CONTROL_TYPE type,
    void* frame_w,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed()) {
    browser_->SetControl(type, ((ExoFrameWrap*)frame_w)->frame_);
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}



void 
ExoBrowserWrap::UnsetControl(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: type */
  ExoBrowser::CONTROL_TYPE type = 
    (ExoBrowser::CONTROL_TYPE) (Local<Integer>::Cast(args[0]))->Value();

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::UnsetControlTask, browser_w, type, cb_p));
}


void
ExoBrowserWrap::UnsetControlTask(
    ExoBrowser::CONTROL_TYPE type,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed()) {
    browser_->UnsetControl(type);
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}


void 
ExoBrowserWrap::SetControlDimension(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: type */
  ExoBrowser::CONTROL_TYPE type = 
    (ExoBrowser::CONTROL_TYPE) (Local<Integer>::Cast(args[0]))->Value();

  /* args[1]: size */
  int size = (Local<Integer>::Cast(args[1]))->Value();

  /* args[2]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[2]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::SetControlDimensionTask, browser_w, 
                 type, size, cb_p));
}


void
ExoBrowserWrap::SetControlDimensionTask(
    ExoBrowser::CONTROL_TYPE type,
    int size,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed()) {
    browser_->SetControlDimension(type, size);
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}





void 
ExoBrowserWrap::AddPage(
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
      /* TODO(spolu): Fix usage of (void*) */
      base::Bind(&ExoBrowserWrap::AddPageTask, browser_w, 
                 (void*)frame_w, cb_p));
}


void
ExoBrowserWrap::AddPageTask(
    void* frame_w,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed()) {
    browser_->AddPage(((ExoFrameWrap*)frame_w)->frame_);
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}




void 
ExoBrowserWrap::RemovePage(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: name */
  std::string name = std::string(
      *String::Utf8Value(Local<String>::Cast(args[0])));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::RemovePageTask, browser_w, name, cb_p));
}


void
ExoBrowserWrap::RemovePageTask(
    const std::string& name,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed()) {
    browser_->RemovePage(name);
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}




void 
ExoBrowserWrap::ShowPage(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: name */
  std::string name = std::string(
      *String::Utf8Value(Local<String>::Cast(args[0])));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::ShowPageTask, browser_w, name, cb_p));
}


void
ExoBrowserWrap::ShowPageTask(
    const std::string& name,
    Persistent<Function>* cb_p)
{
  if(!browser_->is_killed()) {
    browser_->ShowPage(name);
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}

/******************************************************************************/
/*                                DISPATCHERS                                 */
/******************************************************************************/

void
ExoBrowserWrap::SetOpenURLCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->open_url_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchOpenURL(
    const std::string& url,
    const std::string& from_frame)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!open_url_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), open_url_cb_);

    Local<String> url_arg = String::New(url.c_str());
    Local<String> from_frame_arg = String::New(from_frame.c_str());

    Local<Value> argv[2] = { url_arg,
                             from_frame_arg };
    cb->Call(browser_o, 2, argv);
  }
}


void
ExoBrowserWrap::SetResizeCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->resize_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchResize(
    const gfx::Size& size)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!resize_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), resize_cb_);

    Local<Array> size_arg = Array::New();
    size_arg->Set(0, Integer::New(size.width()));
    size_arg->Set(1, Integer::New(size.height()));
    Local<Value> argv[1] = { size_arg };

    cb->Call(browser_o, 1, argv);
  }
}


void
ExoBrowserWrap::SetKillCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->kill_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchKill()
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!kill_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), kill_cb_);

    cb->Call(browser_o, 0, NULL);
  }
}


void
ExoBrowserWrap::SetFrameCloseCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->frame_close_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchFrameClose(
    const std::string& frame)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!frame_close_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), frame_close_cb_);

    Local<String> frame_arg = String::New(frame.c_str());

    Local<Value> argv[1] = { frame_arg };
    cb->Call(browser_o, 1, argv);
  }
}


void
ExoBrowserWrap::SetFrameCreatedCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->frame_created_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchFrameCreated(
    const ExoFrame* frame)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!frame_created_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), frame_created_cb_);

    Local<Object> frame_arg = frame->wrapper_->handle();

    Local<Value> argv[1] = { frame_arg };
    cb->Call(browser_o, 1, argv);
  }
}

void
ExoBrowserWrap::SetFrameKeyboardCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->frame_keyboard_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchFrameKeyboard(
    const std::string& frame,
    const content::NativeWebKeyboardEvent& event)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!frame_keyboard_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), frame_keyboard_cb_);

    Local<String> frame_arg = String::New(frame.c_str());

    Local<Object> event_arg = Object::New();
    event_arg->Set(String::New("type"), Integer::New(event.type));
    event_arg->Set(String::New("modifiers"), Integer::New(event.modifiers));
    event_arg->Set(String::New("keycode"), Integer::New(event.windowsKeyCode));

    Local<Value> argv[2] = { frame_arg,
                             event_arg };
    cb->Call(browser_o, 2, argv);
  }
}


} // namespace breach
    
