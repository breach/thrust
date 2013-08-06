// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/exo_frame_wrap.h"

#include "content/public/browser/browser_thread.h"
#include "breach/browser/ui/exo_browser.h"
#include "breach/browser/ui/exo_frame.h"
#include "breach/browser/node/api/exo_browser_wrap.h"

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
  /*
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setVisible"),
      FunctionTemplate::New(SetVisible)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setSize"),
      FunctionTemplate::New(SetSize)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_setPosition"),
      FunctionTemplate::New(SetPosition)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_size"),
      FunctionTemplate::New(Size)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_position"),
      FunctionTemplate::New(Position)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_loadURL"),
      FunctionTemplate::New(LoadURL)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_goBack"),
      FunctionTemplate::New(GoBack)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_goForward"),
      FunctionTemplate::New(GoForward)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_reload"),
      FunctionTemplate::New(Reload)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_stop"),
      FunctionTemplate::New(Stop)->GetFunction());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("_name"),
      FunctionTemplate::New(Name)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("_parent"),
      FunctionTemplate::New(Parent)->GetFunction());
  */

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewSymbol("_createNewExoFrame"),
      FunctionTemplate::New(CreateNewExoFrame)->GetFunction());
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
ExoFrameWrap::CreateNewExoFrame(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  /* args[0]: spec = { name, url, visible, position, size, zIndex } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  std::string name = std::string(
      String::Utf8Value(spec->Get(String::New("name"))->ToString()))
  std::string url = std::string(
      String::Utf8Value(spec->Get(String::New("url"))->ToString()))
  bool visible = spec->Get(String::New("visible"))->ToBoolean()->Value();
  
  Local<Array> position_array = Array::Cast(spec->Get(String::New("position")));
  gfx::Point position(
      position_array->Get(Integer::New(0))->ToNumber()->Value(),
      position_array->Get(Integer::New(1))->ToNumber()->Value());

  Local<Array> size_array = Array::Cast(spec->Get(String::New("position")));
  gfx::Size size(
      size_array->Get(Integer::New(0))->ToNumber()->Value(),
      size_array->Get(Integer::New(1))->ToNumber()->Value());

  int zIndex = spec->Get(String::New("zIndex"))->ToNumber()->Value();

  /* args[0]: cb_ */
  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> frame_o = c->NewInstance();
  ExoFrameWrap* frame_w = ObjectWrap::Unwrap<ExoFrameWrap>(frame_o);

  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTaskAndReply(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoFrameWrap::CreateTask, frame_w, 
                 name, url, position, size, zIndex),
      base::Bind(&ExoFrameWrap::CreateCallback, frame_w, cb_p));
}


void
ExoFrameWrap::CreateTask(
    std::string& name,
    std::string& url,
    gfx::Point& position,
    gfx::Size& size,
    int zIndex)
{
  /* TODO(spolu): web contents! */
  frame_ = new ExoFrame(name,
                        parent,
                        this);
  parent->AddFrame(frame_);
  frame_->SetPosition(position);
  frame_->SetSize(position);
  //frame_->SetZIndex(zIndex);
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
  if(!frame->parent() != NULL) {
    DCHECK(!frame->parent()->IsKilled())
    frame->parent()->RemoveFrame(frame->name());
  }
  delete frame;
}

/******************************************************************************/
/*                       WRAPPERS, TASKS & CALLBACKS                          */
/******************************************************************************/

} // namespace breach
