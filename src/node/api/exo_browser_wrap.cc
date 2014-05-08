// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "exo_browser/src/node/api/exo_browser_wrap.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/common/ssl_status.h"
#include "content/public/common/page_type.h"

#include "net/cert/cert_status_flags.h"

#include "exo_browser/src/browser/exo_browser.h"
#include "exo_browser/src/browser/exo_frame.h"
#include "exo_browser/src/node/api/exo_frame_wrap.h"
#include "exo_browser/src/node/node_thread.h"

using namespace v8;

namespace {

static Local<Object> 
ObjectFromNavigationEntry(
    const exo_browser::ExoBrowserWrap::NavigationEntry& entry)
{
  Local<Object> entry_o = Object::New(Isolate::GetCurrent());

  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "url"), 
               String::NewFromUtf8(Isolate::GetCurrent(), 
                                   entry.url_.c_str()));
  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "virtual_url"), 
               String::NewFromUtf8(Isolate::GetCurrent(), 
                                   entry.virtual_url_.c_str()));
  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "title"), 
               String::NewFromUtf8(Isolate::GetCurrent(), 
                                   entry.title_.c_str()));
  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "visible"),
               v8::Boolean::New(Isolate::GetCurrent(), entry.visible_));
  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "timestamp"),
               Number::New(Isolate::GetCurrent(), entry.timestamp_));
  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "id"),
               Number::New(Isolate::GetCurrent(), entry.id_));

  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "type"),
               String::NewFromUtf8(Isolate::GetCurrent(),  
                                   entry.type_.c_str()));

  Local<Object> ssl_o = Object::New(Isolate::GetCurrent());

  ssl_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "security_type"),
             String::NewFromUtf8(Isolate::GetCurrent(), 
                                 entry.ssl_security_type_.c_str()));
  ssl_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "cert_status"), 
             Integer::New(Isolate::GetCurrent(), entry.ssl_cert_status_));
  ssl_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "content_status"), 
             Integer::New(Isolate::GetCurrent(), entry.ssl_content_status_));

  entry_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "ssl"), ssl_o);

  return entry_o;
}

static Local<String> 
StringFromWindowOpenDisposition(
    const WindowOpenDisposition disposition)
{
  Local<String> disposition_str;
  switch(disposition) {
    case SUPPRESS_OPEN:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "suppress_open");
      break;
    case CURRENT_TAB:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "current_tab");
      break;
    case SINGLETON_TAB:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "singelton_tab");
      break;
    case NEW_FOREGROUND_TAB:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "new_foreground_tab");
      break;
    case NEW_BACKGROUND_TAB:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "new_background_tab");
      break;
    case NEW_POPUP:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "new_popup");
      break;
    case NEW_WINDOW:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "new_window");
      break;
    case SAVE_TO_DISK:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "save_to_disk");
      break;
    case OFF_THE_RECORD:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "off_the_record");
      break;
    case IGNORE_ACTION:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "ignore_action");
      break;
    default:
      disposition_str = 
        String::NewFromUtf8(Isolate::GetCurrent(), "unknown");
  }
  return disposition_str;
}

}



namespace exo_browser {

ExoBrowserWrap::NavigationEntry::NavigationEntry()
{
}

ExoBrowserWrap::NavigationEntry::~NavigationEntry()
{
}

Persistent<Function> ExoBrowserWrap::s_constructor;

void 
ExoBrowserWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(Isolate::GetCurrent(), 
                                                      New);
  tpl->SetClassName(String::NewFromUtf8(Isolate::GetCurrent(), "_ExoBrowser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  /* Prototype */
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_kill"),
      FunctionTemplate::New(Isolate::GetCurrent(), Kill)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_size"),
      FunctionTemplate::New(Isolate::GetCurrent(), Size)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_position"),
      FunctionTemplate::New(Isolate::GetCurrent(), Position)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_focus"),
      FunctionTemplate::New(Isolate::GetCurrent(), Focus)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_maximize"),
      FunctionTemplate::New(Isolate::GetCurrent(), Maximize)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setTitle"),
      FunctionTemplate::New(Isolate::GetCurrent(), SetTitle)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_addPage"),
      FunctionTemplate::New(Isolate::GetCurrent(), AddPage)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_removePage"),
      FunctionTemplate::New(Isolate::GetCurrent(), RemovePage)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_showPage"),
      FunctionTemplate::New(Isolate::GetCurrent(), ShowPage)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setControl"),
      FunctionTemplate::New(Isolate::GetCurrent(), SetControl)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setControlDimension"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetControlDimension)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_unsetControl"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            UnsetControl)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_showFloating"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            ShowFloating)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_hideFloating"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            HideFloating)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setOpenURLCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetOpenURLCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setResizeCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetResizeCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setKillCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetKillCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setFrameCloseCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetFrameCloseCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setFrameCreatedCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetFrameCreatedCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setFrameKeyboardCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetFrameKeyboardCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setNavigationStateCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetNavigationStateCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setFindReplyCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetFindReplyCallback)->GetFunction());

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewFromUtf8(Isolate::GetCurrent(), "_createExoBrowser"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            CreateExoBrowser)->GetFunction());
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
  HandleScope scope(Isolate::GetCurrent());

  ExoBrowserWrap* browser_w = new ExoBrowserWrap();
  browser_w->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void 
ExoBrowserWrap::CreateExoBrowser(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> browser_o = c->NewInstance();

  /* We keep a Peristent as the object will be returned asynchronously. */
  Persistent<Object> *browser_p = new Persistent<Object>();
  browser_p->Reset(Isolate::GetCurrent(), browser_o);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(browser_o);

  /* args[0]: spec = { size, icon_path } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  Local<Array> in = Local<Array>::Cast(
      spec->Get(String::NewFromUtf8(Isolate::GetCurrent(), "size")));
  gfx::Size size(in->Get(
        Integer::New(Isolate::GetCurrent(), 0))->ToNumber()->Value(),
                 in->Get(
                   Integer::New(Isolate::GetCurrent(), 1))->ToNumber()->Value());
  std::string icon_path = std::string(*String::Utf8Value(
        Local<String>::Cast(spec->Get(
            String::NewFromUtf8(Isolate::GetCurrent(), "icon_path")))));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::CreateTask, browser_w, 
                 size, icon_path, browser_p, cb_p));
}


void
ExoBrowserWrap::CreateTask(
    const gfx::Size& size,
    const std::string& icon_path,
    Persistent<Object>* browser_p,
    Persistent<Function>* cb_p)
{
  browser_ = ExoBrowser::CreateNew(this, size, icon_path);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::CreateCallback, this, browser_p, cb_p));
}

void 
ExoBrowserWrap::CreateCallback(
    Persistent<Object>* browser_p,
    Persistent<Function>* cb_p)
{
  HandleScope scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> browser_o = Local<Object>::New(Isolate::GetCurrent(),
                                               *browser_p);

  LOG(INFO) << "ExoBrowserWrap CreateCallback";
  Local<v8::Value> argv[1] = { browser_o };
  cb->Call(browser_o, 1, argv);

  cb_p->Reset();
  delete cb_p;
  browser_p->Reset();
  delete browser_p;
}

void
ExoBrowserWrap::DeleteTask(
    ExoBrowser* browser)
{
  LOG(INFO) << "ExoBrowserWrap DeleteTask";
  if(browser != NULL && !browser->is_killed())
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
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    browser_->Kill();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}


void
ExoBrowserWrap::Size(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    (*size) = browser_->size();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::SizeCallback, this, cb_p, size));
}



void
ExoBrowserWrap::Position(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    (*position) = browser_->position();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::PointCallback, this, cb_p, position));
}


void 
ExoBrowserWrap::Focus(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::FocusTask, browser_w, cb_p));
}

void
ExoBrowserWrap::FocusTask(
    Persistent<Function>* cb_p)
{
  if(browser_ != NULL && !browser_->is_killed())
    browser_->Focus();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}


void 
ExoBrowserWrap::Maximize(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::MaximizeTask, browser_w, cb_p));
}

void
ExoBrowserWrap::MaximizeTask(
    Persistent<Function>* cb_p)
{
  if(browser_ != NULL && !browser_->is_killed())
    browser_->Maximize();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}



void 
ExoBrowserWrap::SetTitle(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: title */
  std::string title = std::string(
      *String::Utf8Value(Local<String>::Cast(args[0])));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::SetTitleTask, browser_w, title, cb_p));
}

void
ExoBrowserWrap::SetTitleTask(
    const std::string& title,
    Persistent<Function>* cb_p)
{
  if(browser_ != NULL && !browser_->is_killed())
    browser_->SetTitle(title);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}



void 
ExoBrowserWrap::SetControl(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    browser_->SetControl(type, ((ExoFrameWrap*)frame_w)->frame_);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}



void 
ExoBrowserWrap::UnsetControl(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    browser_->UnsetControl(type);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}


void 
ExoBrowserWrap::SetControlDimension(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    browser_->SetControlDimension(type, size);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}



void 
ExoBrowserWrap::ShowFloating(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: frame */
  Local<Object> frame_o = Local<Object>::Cast(args[0]);
  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

  /* args[1]: x */
  int x = (Local<Integer>::Cast(args[1]))->Value();
  /* args[2]: y */
  int y = (Local<Integer>::Cast(args[2]))->Value();
  /* args[3]: width */
  int width = (Local<Integer>::Cast(args[3]))->Value();
  /* args[4]: height */
  int height = (Local<Integer>::Cast(args[4]))->Value();

  /* args[5]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[5]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      /* TODO(spolu): Fix usage of (void*) */
      base::Bind(&ExoBrowserWrap::ShowFloatingTask, browser_w, 
                 (void*)frame_w, x, y, width, height, cb_p));
}


void
ExoBrowserWrap::ShowFloatingTask(
    void* frame_w,
    int x,
    int y,
    int width,
    int height,
    Persistent<Function>* cb_p)
{
  if(browser_ != NULL && !browser_->is_killed())
    browser_->ShowFloating(((ExoFrameWrap*)frame_w)->frame_, 
                           x, y, 
                           width, height);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}

void 
ExoBrowserWrap::HideFloating(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoBrowserWrap::HideFloatingTask, browser_w, cb_p));
}


void
ExoBrowserWrap::HideFloatingTask(
    Persistent<Function>* cb_p)
{
  if(browser_ != NULL && !browser_->is_killed())
    browser_->HideFloating();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}


void 
ExoBrowserWrap::AddPage(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    browser_->AddPage(((ExoFrameWrap*)frame_w)->frame_);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}




void 
ExoBrowserWrap::RemovePage(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    browser_->RemovePage(name);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::EmptyCallback, this, cb_p));
}




void 
ExoBrowserWrap::ShowPage(
        const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  if(browser_ != NULL && !browser_->is_killed())
    browser_->ShowPage(name);

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
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->open_url_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchOpenURL(
    const std::string& url,
    const WindowOpenDisposition disposition,
    const std::string& from_frame)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!open_url_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), open_url_cb_);

    Local<String> url_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                url.c_str());
    Local<String> disposition_arg = 
      StringFromWindowOpenDisposition(disposition);
    Local<String> from_frame_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                       from_frame.c_str());

    Local<Value> argv[3] = { url_arg,
                             disposition_arg,
                             from_frame_arg };
    cb->Call(browser_o, 3, argv);
  }
}


void
ExoBrowserWrap::SetResizeCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->resize_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchResize(
    const gfx::Size& size)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!resize_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), resize_cb_);

    Local<Array> size_arg = Array::New(Isolate::GetCurrent());
    size_arg->Set(0, Integer::New(Isolate::GetCurrent(), size.width()));
    size_arg->Set(1, Integer::New(Isolate::GetCurrent(), size.height()));
    Local<Value> argv[1] = { size_arg };

    cb->Call(browser_o, 1, argv);
  }
}


void
ExoBrowserWrap::SetKillCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->kill_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchKill()
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());
  /* As well as dispatching the event, we delete the browser_ object right */
  /* away and set it to NULL                                               */
  delete browser_;
  browser_ = NULL;

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
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->frame_close_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchFrameClose(
    const std::string& frame)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!frame_close_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), frame_close_cb_);

    Local<String> frame_arg = String::NewFromUtf8(Isolate::GetCurrent(),  
                                                  frame.c_str());

    Local<Value> argv[1] = { frame_arg };
    cb->Call(browser_o, 1, argv);
  }
}


void
ExoBrowserWrap::SetFrameCreatedCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->frame_created_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchFrameCreated(
    const std::string& src_frame,
    const WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos,
    ExoFrame* new_frame)
{
  HandleScope scope(Isolate::GetCurrent());

  if(!frame_created_cb_.IsEmpty()) {

    Local<Function> c = 
      Local<Function>::New(Isolate::GetCurrent(), ExoFrameWrap::s_constructor);
    Local<Object> frame_o = c->NewInstance();

    /* We keep a Peristent as the object will be returned asynchronously. */
    Persistent<Object> *frame_p = new Persistent<Object>();
    frame_p->Reset(Isolate::GetCurrent(), frame_o);

    /* We will need to pass the ExoFrameWrap directly as we won't be able to */
    /* UnWrap it on the UI thread. */
    ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

    content::BrowserThread::PostTask(
        content::BrowserThread::UI, FROM_HERE,
        /* TODO(spolu): Fix usage of (void*) */
        base::Bind(&ExoBrowserWrap::FrameCreatedTask, this, 
                   src_frame, disposition, initial_pos, new_frame, 
                   (void*)frame_w, frame_p));
  }
}

void 
ExoBrowserWrap::FrameCreatedTask(
    const std::string& src_frame,
    const WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos,
    ExoFrame* new_frame,
    void* frame_w,
    Persistent<Object>* frame_p)
{
  ((ExoFrameWrap*)frame_w)->frame_ = new_frame;
  ((ExoFrameWrap*)frame_w)->frame_->wrapper_ = (ExoFrameWrap*)frame_w;

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::FrameCreatedFinish, this, 
                 src_frame, disposition, initial_pos, frame_p));
}

void 
ExoBrowserWrap::FrameCreatedFinish(
    const std::string& src_frame,
    const WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos,
    Persistent<Object>* frame_p)
{
  HandleScope scope(Isolate::GetCurrent());

  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  Local<Object> frame_o = Local<Object>::New(Isolate::GetCurrent(),
                                             *frame_p);

  Local<Function> cb = 
    Local<Function>::New(Isolate::GetCurrent(), frame_created_cb_);

  Local<String> from_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                               src_frame.c_str());
  Local<String> disposition_arg = StringFromWindowOpenDisposition(disposition);

  Local<Array> initial_pos_arg = Array::New(Isolate::GetCurrent());
  initial_pos_arg->Set(0, Integer::New(Isolate::GetCurrent(), 
                                       initial_pos.x()));
  initial_pos_arg->Set(1, Integer::New(Isolate::GetCurrent(), 
                                       initial_pos.y()));
  initial_pos_arg->Set(2, Integer::New(Isolate::GetCurrent(), 
                                       initial_pos.width()));
  initial_pos_arg->Set(3, Integer::New(Isolate::GetCurrent(), 
                                       initial_pos.height()));

  Local<Value> argv[4] = { frame_o,
                           disposition_arg,
                           initial_pos_arg,
                           from_arg };
  cb->Call(browser_o, 4, argv);

  frame_p->Reset();
  delete frame_p;
}

void
ExoBrowserWrap::SetFrameKeyboardCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

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
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!frame_keyboard_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), frame_keyboard_cb_);

    Local<String> frame_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                  frame.c_str());

    Local<Object> event_arg = Object::New(Isolate::GetCurrent());
    event_arg->Set(String::NewFromUtf8(Isolate::GetCurrent(), "type"), 
                   Integer::New(Isolate::GetCurrent(), event.type));
    event_arg->Set(String::NewFromUtf8(Isolate::GetCurrent(), "modifiers"), 
                   Integer::New(Isolate::GetCurrent(), event.modifiers));
    event_arg->Set(String::NewFromUtf8(Isolate::GetCurrent(), "keycode"), 
                   Integer::New(Isolate::GetCurrent(), event.windowsKeyCode));

    Local<Value> argv[2] = { frame_arg,
                             event_arg };
    cb->Call(browser_o, 2, argv);
  }
}

void
ExoBrowserWrap::SetNavigationStateCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->navigation_state_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchNavigationState(
    const std::string& frame,
    const std::vector<NavigationEntry>& entries,
    bool can_go_back,
    bool can_go_forward)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!navigation_state_cb_.IsEmpty()) {
    Local<Object> state_arg = Object::New(Isolate::GetCurrent());
    
    Local<Array> entries_a = Array::New(Isolate::GetCurrent());
    for(unsigned int i = 0; i < entries.size(); i++) {
      entries_a->Set(Integer::New(Isolate::GetCurrent(), i), 
                     ObjectFromNavigationEntry(entries[i]));
    }
    state_arg->Set(String::NewFromUtf8(Isolate::GetCurrent(), "entries"), 
                   entries_a);
    state_arg->Set(String::NewFromUtf8(Isolate::GetCurrent(), "can_go_back"),
                   v8::Boolean::New(Isolate::GetCurrent(), can_go_back));
    state_arg->Set(String::NewFromUtf8(Isolate::GetCurrent(), "can_go_forward"),
                   v8::Boolean::New(Isolate::GetCurrent(), can_go_forward));

    Local<String> frame_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                  frame.c_str());

    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), navigation_state_cb_);

    Local<Value> argv[2] = { frame_arg,
                             state_arg };
    cb->Call(browser_o, 2, argv);
  }
}


void
ExoBrowserWrap::SetFindReplyCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoBrowserWrap* browser_w = ObjectWrap::Unwrap<ExoBrowserWrap>(args.This());
  browser_w->find_reply_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoBrowserWrap::DispatchFindReply(
    const std::string& src_frame,
    int request_id,
    int number_of_matches,
    const gfx::Rect& selection_rect,
    int active_match_ordinal,
    bool final_update)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> browser_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!find_reply_cb_.IsEmpty()) {
    Local<String> frame_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                  src_frame.c_str());
    Local<Integer> request_id_arg = Integer::New(Isolate::GetCurrent(), 
                                                 request_id);
    Local<Integer> number_of_matches_arg = Integer::New(Isolate::GetCurrent(), 
                                                        number_of_matches);

    Local<Array> selection_rect_arg = Array::New(Isolate::GetCurrent());
    selection_rect_arg->Set(0, Integer::New(Isolate::GetCurrent(), 
                                            selection_rect.x()));
    selection_rect_arg->Set(1, Integer::New(Isolate::GetCurrent(), 
                                            selection_rect.y()));
    selection_rect_arg->Set(2, Integer::New(Isolate::GetCurrent(), 
                                            selection_rect.width()));
    selection_rect_arg->Set(3, Integer::New(Isolate::GetCurrent(), 
                                            selection_rect.height()));

    Local<Integer> active_match_arg = Integer::New(Isolate::GetCurrent(), 
                                                   active_match_ordinal);
    Local<v8::Boolean> final_update_arg = 
      v8::Boolean::New(Isolate::GetCurrent(), final_update);

    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), find_reply_cb_);


    Local<Value> argv[6] = { frame_arg,
                             request_id_arg,
                             number_of_matches_arg,
                             selection_rect_arg,
                             active_match_arg,
                             final_update_arg };

    cb->Call(browser_o, 6, argv);
  }
}


} // namespace exo_browser
    
