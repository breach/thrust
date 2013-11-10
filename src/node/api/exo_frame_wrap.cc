// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "exo_browser/src/node/api/exo_frame_wrap.h"

#include "base/strings/utf_string_conversions.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/favicon_url.h"
#include "content/public/common/context_menu_params.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/render_view_host.h" 
#include "exo_browser/src/browser/ui/exo_browser.h"
#include "exo_browser/src/browser/ui/exo_frame.h"
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
  Local<Object> params_o = Object::New();

  params_o->Set(String::New("x"), Number::New(params.x));
  params_o->Set(String::New("y"), Number::New(params.y));

  params_o->Set(String::New("link_url"),
                String::New(params.link_url.spec().c_str()));
  params_o->Set(String::New("raw_link_url"),
                String::New(params.unfiltered_link_url.spec().c_str()));
  params_o->Set(String::New("src_url"),
                String::New(params.src_url.spec().c_str()));

  params_o->Set(String::New("page_url"),
                String::New(params.page_url.spec().c_str()));
  params_o->Set(String::New("frame_url"),
                String::New(params.frame_url.spec().c_str()));

  params_o->Set(String::New("is_editable"),
                v8::Boolean::New(params.is_editable));

  switch(params.media_type) {
    case WebKit::WebContextMenuData::MediaTypeImage:
      params_o->Set(String::New("media_type"), String::New("image"));
      break;
    case WebKit::WebContextMenuData::MediaTypeVideo:
      params_o->Set(String::New("media_type"), String::New("video"));
      break;
    case WebKit::WebContextMenuData::MediaTypeAudio:
      params_o->Set(String::New("media_type"), String::New("audio"));
      break;
    case WebKit::WebContextMenuData::MediaTypeFile:
      params_o->Set(String::New("media_type"), String::New("file"));
      break;
    case WebKit::WebContextMenuData::MediaTypePlugin:
      params_o->Set(String::New("media_type"), String::New("plugin"));
      break;
    default:
    case WebKit::WebContextMenuData::MediaTypeNone:
      params_o->Set(String::New("media_type"), String::New("none"));
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

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_undo"),
      FunctionTemplate::New(Undo)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_redo"),
      FunctionTemplate::New(Redo)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_cutSelection"),
      FunctionTemplate::New(CutSelection)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_copySelection"),
      FunctionTemplate::New(CopySelection)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_paste"),
      FunctionTemplate::New(Paste)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_deleteSelection"),
      FunctionTemplate::New(DeleteSelection)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_selectAll"),
      FunctionTemplate::New(SelectAll)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_unselect"),
      FunctionTemplate::New(Unselect)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_focus"),
      FunctionTemplate::New(Focus)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_find"),
      FunctionTemplate::New(Find)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_stopFinding"),
      FunctionTemplate::New(StopFinding)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_capture"),
      FunctionTemplate::New(Capture)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_zoom"),
      FunctionTemplate::New(Zoom)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_zoomLevel"),
      FunctionTemplate::New(ZoomLevel)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_name"),
      FunctionTemplate::New(Name)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_type"),
      FunctionTemplate::New(Type)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("_setBuildContextMenuHandler"),
      FunctionTemplate::New(SetBuildContextMenuHandler)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setLoadFailCallback"),
      FunctionTemplate::New(SetLoadFailCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setLoadFinishCallback"),
      FunctionTemplate::New(SetLoadFinishCallback)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setLoadingStartCallback"),
      FunctionTemplate::New(SetLoadingStartCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setLoadingStopCallback"),
      FunctionTemplate::New(SetLoadingStopCallback)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setFaviconUpdateCallback"),
      FunctionTemplate::New(SetFaviconUpdateCallback)->GetFunction());

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

  /* We keep a Peristent as the object will be returned asynchronously. */
  Persistent<Object> *frame_p = new Persistent<Object>();
  frame_p->Reset(Isolate::GetCurrent(), frame_o);

  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

  /* args[0]: spec = { name, url, session } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  std::string name = std::string(
      *String::Utf8Value(spec->Get(String::New("name"))->ToString()));
  std::string url = std::string(
      *String::Utf8Value(spec->Get(String::New("url"))->ToString()));
  Local<Object> session_o = 
    Local<Object>::Cast(spec->Get(String::New("session")));
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

  cb_p->Dispose();
  delete cb_p;
  frame_p->Dispose();
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
  if(frame_ != NULL)
    frame_->web_contents_->GetRenderViewHost()->Undo();

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
  if(frame_ != NULL)
    frame_->web_contents_->GetRenderViewHost()->Redo();

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
  if(frame_ != NULL) 
    frame_->web_contents_->GetRenderViewHost()->Cut();

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
  if(frame_ != NULL) 
    frame_->web_contents_->GetRenderViewHost()->Copy();

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
  if(frame_ != NULL)
    frame_->web_contents_->GetRenderViewHost()->Paste();

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
  if(frame_ != NULL)
    frame_->web_contents_->GetRenderViewHost()->Delete();

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
  if(frame_ != NULL)
    frame_->web_contents_->GetRenderViewHost()->SelectAll();

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
  if(frame_ != NULL)
    frame_->web_contents_->GetRenderViewHost()->Unselect();

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
  WebKit::WebFindOptions options;
  options.forward = forward;
  options.matchCase = matchCase;
  options.findNext = findNext;

  string16 text = UTF8ToUTF16(search_text);

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
        *String::Utf8Value(Local<String>::Cast(items->Get(Integer::New(i)))));
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
      FunctionTemplate::New(BuildContextMenuCallback);

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

    Local<Integer> index_arg = Integer::New(index);

    Local<Value> argv[1] = { index_arg };
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

    Local<String> url_arg = String::New(url.c_str());
    Local<Integer> error_code_arg = Integer::New(error_code);
    Local<String> error_desc_arg = String::New(error_desc.c_str());

    Local<Value> argv[3] = { url_arg,
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

    Local<String> url_arg = String::New(url.c_str());

    Local<Value> argv[1] = { url_arg };
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

    Local<Array> favicons_arg = Array::New();
    for(unsigned int i = 0; i < candidates.size(); i ++) {
      if(candidates[i].icon_type == content::FaviconURL::INVALID_ICON) {
        favicons_arg->Set(Integer::New(i), Null());
      }
      else {
        favicons_arg->Set(Integer::New(i), 
            String::New(candidates[i].icon_url.spec().c_str()));
      }
    };

    Local<Value> argv[1] = { favicons_arg };
    cb->Call(frame_o, 1, argv);
  }
}

} // namespace exo_browser

