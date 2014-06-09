// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "exo_browser/src/node/api/exo_frame_wrap.h"

#include "base/strings/utf_string_conversions.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/context_menu_params.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_frame_host.h" 
#include "content/public/browser/render_view_host.h" 
#include "exo_browser/src/browser/exo_browser.h"
#include "exo_browser/src/browser/exo_frame.h"
#include "exo_browser/src/node/api/exo_browser_wrap.h"
#include "exo_browser/src/node/api/exo_session_wrap.h"
#include "exo_browser/src/node/node_thread.h"
#include "third_party/WebKit/public/web/WebFindOptions.h"


using namespace v8;

namespace {

static Local<Object>
ObjectFromContextMenuParams(
    const content::ContextMenuParams params)
{
  Local<Object> params_o = Object::New(Isolate::GetCurrent());

  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "x"), 
                Number::New(Isolate::GetCurrent(), params.x));
  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "y"), 
                Number::New(Isolate::GetCurrent(), params.y));

  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "link_url"),
                String::NewFromUtf8(Isolate::GetCurrent(), 
                                    params.link_url.spec().c_str()));
  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "raw_link_url"),
                String::NewFromUtf8(Isolate::GetCurrent(), 
                                    params.unfiltered_link_url.spec().c_str()));
  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "src_url"),
                String::NewFromUtf8(Isolate::GetCurrent(), 
                                    params.src_url.spec().c_str()));

  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "page_url"),
                String::NewFromUtf8(Isolate::GetCurrent(), 
                                    params.page_url.spec().c_str()));
  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "frame_url"),
                String::NewFromUtf8(Isolate::GetCurrent(), 
                                    params.frame_url.spec().c_str()));

  params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "is_editable"),
                v8::Boolean::New(Isolate::GetCurrent(), params.is_editable));

  switch(params.media_type) {
    case blink::WebContextMenuData::MediaTypeImage:
      params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "media_type"), 
                    String::NewFromUtf8(Isolate::GetCurrent(), "image"));
      break;
    case blink::WebContextMenuData::MediaTypeVideo:
      params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "media_type"), 
                    String::NewFromUtf8(Isolate::GetCurrent(), "video"));
      break;
    case blink::WebContextMenuData::MediaTypeAudio:
      params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "media_type"), 
                    String::NewFromUtf8(Isolate::GetCurrent(), "audio"));
      break;
    case blink::WebContextMenuData::MediaTypeFile:
      params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "media_type"), 
                    String::NewFromUtf8(Isolate::GetCurrent(), "file"));
      break;
    case blink::WebContextMenuData::MediaTypePlugin:
      params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "media_type"), 
                    String::NewFromUtf8(Isolate::GetCurrent(), "plugin"));
      break;
    default:
    case blink::WebContextMenuData::MediaTypeNone:
      params_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "media_type"), 
                    String::NewFromUtf8(Isolate::GetCurrent(), "none"));
      break;
  }

  return params_o;
}

}

namespace exo_browser {

Persistent<Function> ExoFrameWrap::s_constructor;

void 
ExoFrameWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(Isolate::GetCurrent(), 
                                                      New);
  tpl->SetClassName(String::NewFromUtf8(Isolate::GetCurrent(), "_ExoFrame"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  /* Prototype */
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_detach"),
      FunctionTemplate::New(Isolate::GetCurrent(), Detach)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_loadURL"),
      FunctionTemplate::New(Isolate::GetCurrent(), LoadURL)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_goBackOrForward"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            GoBackOrForward)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_reload"),
      FunctionTemplate::New(Isolate::GetCurrent(), Reload)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_stop"),
      FunctionTemplate::New(Isolate::GetCurrent(), Stop)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_undo"),
      FunctionTemplate::New(Isolate::GetCurrent(), Undo)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_redo"),
      FunctionTemplate::New(Isolate::GetCurrent(), Redo)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_cutSelection"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            CutSelection)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_copySelection"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            CopySelection)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_paste"),
      FunctionTemplate::New(Isolate::GetCurrent(), Paste)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_deleteSelection"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            DeleteSelection)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_selectAll"),
      FunctionTemplate::New(Isolate::GetCurrent(), SelectAll)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_unselect"),
      FunctionTemplate::New(Isolate::GetCurrent(), Unselect)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_focus"),
      FunctionTemplate::New(Isolate::GetCurrent(), Focus)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_find"),
      FunctionTemplate::New(Isolate::GetCurrent(), Find)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_stopFinding"),
      FunctionTemplate::New(Isolate::GetCurrent(), StopFinding)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_capture"),
      FunctionTemplate::New(Isolate::GetCurrent(), Capture)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_getDevToolsId"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            GetDevToolsId)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_zoom"),
      FunctionTemplate::New(Isolate::GetCurrent(), Zoom)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_zoomLevel"),
      FunctionTemplate::New(Isolate::GetCurrent(), ZoomLevel)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_name"),
      FunctionTemplate::New(Isolate::GetCurrent(), Name)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_type"),
      FunctionTemplate::New(Isolate::GetCurrent(), Type)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setBuildContextMenuHandler"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetBuildContextMenuHandler)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setLoadFailCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetLoadFailCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setLoadFinishCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetLoadFinishCallback)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setLoadingStartCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetLoadingStartCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setLoadingStopCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetLoadingStopCallback)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setFaviconUpdateCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetFaviconUpdateCallback)->GetFunction());

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewFromUtf8(Isolate::GetCurrent(), "_createExoFrame"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            CreateExoFrame)->GetFunction());
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

  /* We keep a Peristent as the object will be returned asynchronously. */
  Persistent<Object> *frame_p = new Persistent<Object>();
  frame_p->Reset(Isolate::GetCurrent(), frame_o);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

  /* args[0]: spec = { name, url, session } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  std::string name = std::string(
      *String::Utf8Value(spec->Get(
          String::NewFromUtf8(Isolate::GetCurrent(), "name"))->ToString()));
  std::string url = std::string(
      *String::Utf8Value(spec->Get(
          String::NewFromUtf8(Isolate::GetCurrent(), "url"))->ToString()));
  Local<Object> session_o = 
    Local<Object>::Cast(spec->Get(
          String::NewFromUtf8(Isolate::GetCurrent(), "session")));
  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(session_o);
  /* We keep the ExoSessionWrap as is and suppose here that the object won't */
  /* be reclaimed for the time of the ExoFrame construction. The API wrapper */
  /* must ensure that.                                                       */
  
  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::CreateTask, frame_w, 
                 name, url, (void*)session_w, frame_p, cb_p));
}


void
ExoFrameWrap::CreateTask(
    const std::string& name,
    const std::string& url,
    void* session_w,
    Persistent<Object>* frame_p,
    Persistent<Function>* cb_p)
{
  frame_ = new ExoFrame(name,
                        ((ExoSessionWrap*)session_w)->session_,
                        this);
  frame_->LoadURL(GURL(url));

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::CreateCallback, this, frame_p, cb_p));
}

void
ExoFrameWrap::CreateCallback(
    Persistent<Object>* frame_p,
    Persistent<Function>* cb_p)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> frame_o = Local<Object>::New(Isolate::GetCurrent(),
                                             *frame_p);

  LOG(INFO) << "CreateCallback ExoFrameWrap";
  Local<v8::Value> argv[1] = { frame_o };
  cb->Call(frame_o, 1, argv);

  cb_p->Reset();
  delete cb_p;
  frame_p->Reset();
  delete frame_p;
}


void
ExoFrameWrap::DeleteTask(
    ExoFrame* frame)
{
  LOG(INFO) << "ExoFrameWrap DeleteTask";
  if(frame != NULL && frame->parent() != NULL) {
    DCHECK(!frame->parent()->is_killed());
    frame->parent()->RemoveFrame(frame->name());
  }
  delete frame;
}

/******************************************************************************/
/* WRAPPERS, TASKS */
/******************************************************************************/
void 
ExoFrameWrap::Detach(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::DetachTask, frame_w, cb_p));
}


void
ExoFrameWrap::DetachTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    if(frame_ != NULL && frame_->parent() != NULL) {
      DCHECK(!frame_->parent()->is_killed());
      frame_->parent()->RemoveFrame(frame_->name());
    }
    delete frame_;
    frame_ = NULL;
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}

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

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::LoadURLTask, frame_w, url, cb_p));
}


void
ExoFrameWrap::LoadURLTask(
    const std::string& url,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    frame_->LoadURL(GURL(url));

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
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
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::GoBackOrForwardTask, frame_w, offset, cb_p));
}


void
ExoFrameWrap::GoBackOrForwardTask(
    int offset,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    frame_->GoBackOrForward(offset);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
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
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::ReloadTask, frame_w, cb_p));
}


void
ExoFrameWrap::ReloadTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    frame_->Reload();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
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
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::StopTask, frame_w, cb_p));
}


void
ExoFrameWrap::StopTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    frame_->Stop();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}



void 
ExoFrameWrap::Undo(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::UndoTask, frame_w, cb_p));
}


void
ExoFrameWrap::UndoTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->Undo();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}

void 
ExoFrameWrap::Redo(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::RedoTask, frame_w, cb_p));
}


void
ExoFrameWrap::RedoTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->Redo();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}


void 
ExoFrameWrap::CutSelection(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::CutSelectionTask, frame_w, cb_p));
}


void
ExoFrameWrap::CutSelectionTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->Cut();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}

void 
ExoFrameWrap::CopySelection(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::CopySelectionTask, frame_w, cb_p));
}


void
ExoFrameWrap::CopySelectionTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->Copy();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}


void 
ExoFrameWrap::Paste(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::PasteTask, frame_w, cb_p));
}


void
ExoFrameWrap::PasteTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->Paste();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}

void 
ExoFrameWrap::DeleteSelection(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::DeleteSelectionTask, frame_w, cb_p));
}


void
ExoFrameWrap::DeleteSelectionTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->Delete();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}

void 
ExoFrameWrap::SelectAll(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::SelectAllTask, frame_w, cb_p));
}


void
ExoFrameWrap::SelectAllTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->SelectAll();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}


void 
ExoFrameWrap::Unselect(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::UnselectTask, frame_w, cb_p));
}


void
ExoFrameWrap::UnselectTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    content::RenderFrameHost* f = frame_->web_contents_->GetFocusedFrame();
    if(f) {
      f->Unselect();
    }
  }

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}
void 
ExoFrameWrap::Focus(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::FocusTask, frame_w, cb_p));
}


void
ExoFrameWrap::FocusTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    frame_->Focus();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}


void 
ExoFrameWrap::Find(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: request_id */
  int request_id = (Local<Integer>::Cast(args[0]))->Value();

  /* args[1]: search_text */
  std::string search_text = std::string(
      *String::Utf8Value(Local<String>::Cast(args[1])));

  /* args[2]: forward */
  bool forward = args[2]->BooleanValue();

  /* args[3]: matchCase */
  bool matchCase = args[3]->BooleanValue();

  /* args[4]: findNext */
  bool findNext = args[4]->BooleanValue();

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[5]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::FindTask, frame_w, 
                 request_id, search_text, forward, matchCase, findNext, cb_p));
}


void
ExoFrameWrap::FindTask(
    int request_id, const std::string& search_text,
    bool forward, bool matchCase, bool findNext,
    Persistent<Function>* cb_p)
{
  blink::WebFindOptions options;
  options.forward = forward;
  options.matchCase = matchCase;
  options.findNext = findNext;

  base::string16 text = base::UTF8ToUTF16(search_text);

  if(frame_ != NULL)
    frame_->Find(request_id, text, options);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}


void 
ExoFrameWrap::StopFinding(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: action */
  std::string action_str = std::string(
      *String::Utf8Value(Local<String>::Cast(args[0])));

  content::StopFindAction action = content::STOP_FIND_ACTION_CLEAR_SELECTION;
  if(action_str.compare("clear") == 0) {
    action = content::STOP_FIND_ACTION_CLEAR_SELECTION;
  }
  if(action_str.compare("keep") == 0) {
    action = content::STOP_FIND_ACTION_KEEP_SELECTION;
  }
  if(action_str.compare("activate") == 0) {
    action = content::STOP_FIND_ACTION_ACTIVATE_SELECTION;
  }

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::StopFindingTask, frame_w, action, cb_p));
}


void
ExoFrameWrap::StopFindingTask(
    content::StopFindAction action,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    frame_->StopFinding(action);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}



void 
ExoFrameWrap::Capture(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::CaptureTask, frame_w, cb_p));
}

void
ExoFrameWrap::CaptureTask(
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL) {
    frame_->CaptureFrame(
        base::Bind(&ExoFrameWrap::CaptureCallback, this, cb_p));
  }
}

void
ExoFrameWrap::CaptureCallback(
    Persistent<Function>* cb_p,
    bool suceeded,
    const std::string& result)
{
  std::string* data = new std::string();
  (*data) = result;

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::StringCallback, this, cb_p, data));
}

void
ExoFrameWrap::GetDevToolsId(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  std::string* id = new std::string();

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::GetDevToolsIdTask, frame_w, id, cb_p));
}

void
ExoFrameWrap::GetDevToolsIdTask(
    std::string* id,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    (*id) = frame_->GetDevToolsId();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::StringCallback, this, cb_p, id));
}

void 
ExoFrameWrap::Zoom(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: zoom */
  std::string zoom_str = std::string(
      *String::Utf8Value(Local<String>::Cast(args[0])));

  content::PageZoom zoom = content::PAGE_ZOOM_RESET;
  if(zoom_str.compare("in") == 0) {
    zoom = content::PAGE_ZOOM_IN;
  }
  if(zoom_str.compare("reset") == 0) {
    zoom = content::PAGE_ZOOM_RESET;
  }
  if(zoom_str.compare("out") == 0) {
    zoom = content::PAGE_ZOOM_OUT;
  }

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::ZoomTask, frame_w, zoom, cb_p));
}


void
ExoFrameWrap::ZoomTask(
    const content::PageZoom zoom,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    frame_->Zoom(zoom);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::EmptyCallback, this, cb_p));
}

void 
ExoFrameWrap::ZoomLevel(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  double* number = new double;

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::ZoomLevelTask, frame_w, number, cb_p));
}


void
ExoFrameWrap::ZoomLevelTask(
    double* number,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    (*number) = frame_->ZoomLevel();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::DoubleCallback, this, cb_p, number));
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
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::NameTask, frame_w, name, cb_p));
}


void
ExoFrameWrap::NameTask(
    std::string* name,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    (*name) = frame_->name();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::StringCallback, this, cb_p, name));
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
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::TypeTask, frame_w, type, cb_p));
}


void
ExoFrameWrap::TypeTask(
    int* type,
    Persistent<Function>* cb_p)
{
  if(frame_ != NULL)
    (*type) = (int)frame_->type();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::IntCallback, this, cb_p, type));
}

/******************************************************************************/
/* HANDLERS */
/******************************************************************************/

void ExoFrameWrap::SetBuildContextMenuHandler(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: hdlr */
  Local<Function> hdlr = Local<Function>::Cast(args[0]);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  frame_w->build_context_menu_hdlr_.Reset(Isolate::GetCurrent(), hdlr);
}

void ExoFrameWrap::BuildContextMenuCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  ExoFrameWrap* frame_w = 
    ObjectWrap::Unwrap<ExoFrameWrap>(args.This());

  /* args[0]: items */
  std::vector<std::string> menu;
  Local<Array> items = Local<Array>::Cast(args[0]);
  for(unsigned int i = 0; i < items->Length(); i ++) {
    std::string item = std::string(
        *String::Utf8Value(Local<String>::Cast(
            items->Get(Integer::New(Isolate::GetCurrent(), i)))));
    menu.push_back(item);
  }

  /* args[1]: trigger_ */
  Local<Function> trigger = Local<Function>::Cast(args[1]);
  frame_w->build_context_menu_trigger_.Reset(Isolate::GetCurrent(), trigger);

  //LOG(INFO) << "BuildContextMenuCallback" << " [" << menu.size() << "]";

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(frame_w->build_context_menu_callback_, menu, 
                 base::Bind(&ExoFrameWrap::CallTriggerContextMenuItem, 
                            frame_w)));
}

void ExoFrameWrap::CallBuildContextMenu(
    const content::ContextMenuParams& params,
    const ContextMenuCallback& cb)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!build_context_menu_hdlr_.IsEmpty()) {
    Local<Function> hdlr = 
      Local<Function>::New(Isolate::GetCurrent(), build_context_menu_hdlr_);

    build_context_menu_callback_ = cb;
    Local<FunctionTemplate> tpl = 
      FunctionTemplate::New(Isolate::GetCurrent(), BuildContextMenuCallback);

    Local<Object> params_arg = ObjectFromContextMenuParams(params);
    Local<Function> cb_arg = tpl->GetFunction();

    Local<v8::Value> argv[2] = { params_arg,
                                 cb_arg };
    hdlr->Call(frame_o, 2, argv);
  }
}

void ExoFrameWrap::CallTriggerContextMenuItem(
    const int index)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!build_context_menu_trigger_.IsEmpty()) {
    Local<Function> trigger = 
      Local<Function>::New(Isolate::GetCurrent(), build_context_menu_trigger_);

    Local<Integer> index_arg = Integer::New(Isolate::GetCurrent(), index);

    Local<v8::Value> argv[1] = { index_arg };
    trigger->Call(frame_o, 1, argv);
  }
}

/******************************************************************************/
/* DISPATCHERS */
/******************************************************************************/

void
ExoFrameWrap::SetLoadFailCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  frame_w->load_fail_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoFrameWrap::DispatchLoadFail(
    const std::string& url,
    const int error_code,
    const std::string& error_desc)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!load_fail_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), load_fail_cb_);

    Local<String> url_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                url.c_str());
    Local<Integer> error_code_arg = Integer::New(Isolate::GetCurrent(), 
                                                 error_code);
    Local<String> error_desc_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                       error_desc.c_str());

    Local<v8::Value> argv[3] = { url_arg,
                             error_code_arg,
                             error_desc_arg };
    cb->Call(frame_o, 3, argv);
  }
}

void
ExoFrameWrap::SetLoadFinishCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  frame_w->load_finish_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoFrameWrap::DispatchLoadFinish(
    const std::string& url)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!load_finish_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), load_finish_cb_);

    Local<String> url_arg = String::NewFromUtf8(Isolate::GetCurrent(), 
                                                url.c_str());

    Local<v8::Value> argv[1] = { url_arg };
    cb->Call(frame_o, 1, argv);
  }
}


void
ExoFrameWrap::SetLoadingStartCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  frame_w->loading_start_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoFrameWrap::DispatchLoadingStart()
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!loading_start_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), loading_start_cb_);

    cb->Call(frame_o, 0, NULL);
  }
}

void
ExoFrameWrap::SetLoadingStopCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  frame_w->loading_stop_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoFrameWrap::DispatchLoadingStop()
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!loading_stop_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), loading_stop_cb_);

    cb->Call(frame_o, 0, NULL);
  }
}

void
ExoFrameWrap::SetFaviconUpdateCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(args.This());
  frame_w->favicon_update_cb_.Reset(Isolate::GetCurrent(), cb);
}

void
ExoFrameWrap::DispatchFaviconUpdate(
    const std::vector<content::FaviconURL>& candidates)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  Local<Object> frame_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!favicon_update_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), favicon_update_cb_);

    Local<Array> favicons_arg = Array::New(Isolate::GetCurrent());
    for(unsigned int i = 0; i < candidates.size(); i ++) {
      if(candidates[i].icon_type == content::FaviconURL::INVALID_ICON) {
        favicons_arg->Set(Integer::New(Isolate::GetCurrent(), i), 
                          Null(Isolate::GetCurrent()));
      }
      else {
        favicons_arg->Set(Integer::New(Isolate::GetCurrent(), i), 
            String::NewFromUtf8(Isolate::GetCurrent(), 
                                candidates[i].icon_url.spec().c_str()));
      }
    };

    Local<v8::Value> argv[1] = { favicons_arg };
    cb->Call(frame_o, 1, argv);
  }
}

} // namespace exo_browser

