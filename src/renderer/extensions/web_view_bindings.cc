// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2013 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/extensions/web_view_bindings.h"

#include <string>

#include "base/bind.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "v8/include/v8.h"
#include "content/public/renderer/v8_value_converter.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"

#include "src/renderer/extensions/script_context.h"
#include "src/common/messages.h"
#include "src/renderer/render_frame_observer.h"

using namespace content;

namespace extensions {

WebViewBindings::WebViewBindings(
    ScriptContext* context)
  : ObjectBackedNativeHandler(context) 
{
  RouteFunction("CreateGuest",
      base::Bind(&WebViewBindings::CreateGuest,
                 base::Unretained(this)));
  RouteFunction("DestroyGuest",
      base::Bind(&WebViewBindings::DestroyGuest,
                 base::Unretained(this)));
  RouteFunction("SetEventHandler",
      base::Bind(&WebViewBindings::SetEventHandler,
                 base::Unretained(this)));
  RouteFunction("SetAutoSize",
      base::Bind(&WebViewBindings::SetAutoSize,
                 base::Unretained(this)));
  RouteFunction("LoadUrl",
      base::Bind(&WebViewBindings::LoadUrl,
                 base::Unretained(this)));
  RouteFunction("Go",
      base::Bind(&WebViewBindings::Go,
                 base::Unretained(this)));
  RouteFunction("Reload",
      base::Bind(&WebViewBindings::Reload,
                 base::Unretained(this)));

  render_frame_observer_ = 
    thrust_shell::ThrustShellRenderFrameObserver::FromRenderFrame(
        RenderFrame::FromWebFrame(this->context()->web_frame()));
  render_frame_observer_->AddWebViewBindings(this);

  LOG(INFO) << "WebViewBindings Constructor " << this;
}

WebViewBindings::~WebViewBindings()
{
  render_frame_observer_->RemoveWebViewBindings(this);

  LOG(INFO) << "WebViewBindings Destructor " << this;
}

bool 
WebViewBindings::AttemptEmitEvent(
    int guest_instance_id,
    const std::string type,
    const base::DictionaryValue& event)
{
  if(guest_handlers_.find(guest_instance_id) != guest_handlers_.end()) {
    v8::HandleScope handle_scope(context()->isolate());

    v8::Local<v8::String> type_arg = 
      v8::String::NewFromUtf8(context()->isolate(), type.c_str());
    scoped_ptr<V8ValueConverter> converter(V8ValueConverter::create());
    v8::Handle<v8::Value> event_arg = converter->ToV8Value(&event, context()->v8_context());

    v8::Local<v8::Function> handler = 
            v8::Local<v8::Function>::New(context()->isolate(),
                                         guest_handlers_[guest_instance_id]);

    v8::Local<v8::Value> argv[2] = { type_arg,
                                     event_arg };
    context()->CallFunction(handler, 2, argv);
  }
  return false;
}

void 
WebViewBindings::CreateGuest(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if(args.Length() != 1 || !args[0]->IsObject()) {
    NOTREACHED();
    return;
  }

  v8::Local<v8::Object> object = args[0]->ToObject();

  scoped_ptr<V8ValueConverter> converter(V8ValueConverter::create());
  scoped_ptr<base::Value> value(
      converter->FromV8Value(object, context()->v8_context()));

  if(!value) {
    return;                                                             
  }
  if(!value->IsType(base::Value::TYPE_DICTIONARY)) {
    return;
  }

  scoped_ptr<base::DictionaryValue> params(
      static_cast<base::DictionaryValue*>(value.release()));

  LOG(INFO) << "WEB_VIEW_BINDINGS: CreateGuest";

  int id = 0;
  render_frame_observer_->Send(
      new ThrustFrameHostMsg_CreateWebViewGuest(
        render_frame_observer_->routing_id(), 
        *params.get(), &id));

  args.GetReturnValue().Set(v8::Integer::New(context()->isolate(), id));
}

void 
WebViewBindings::DestroyGuest(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if(args.Length() != 1 || !args[0]->IsNumber()) {
    NOTREACHED();
    return;
  }

  int guest_instance_id = args[0]->NumberValue();
  LOG(INFO) << "WEB_VIEW_BINDINGS: DestroyGuest " << guest_instance_id;
  
  render_frame_observer_->Send(
      new ThrustFrameHostMsg_DestroyWebViewGuest(
        render_frame_observer_->routing_id(), 
        guest_instance_id));
}

void 
WebViewBindings::SetEventHandler(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if(args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsFunction()) {
    NOTREACHED();
    return;
  }

  int guest_instance_id = args[0]->NumberValue();
  LOG(INFO) << "WEB_VIEW_BINDINGS: SetEventHandler " << guest_instance_id;

  v8::Local<v8::Function> cb = v8::Local<v8::Function>::Cast(args[1]);
  guest_handlers_[guest_instance_id].Reset(context()->isolate(), cb);
}

void 
WebViewBindings::SetAutoSize(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  if(args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsObject()) {
    NOTREACHED();
    return;
  }

  int guest_instance_id = args[0]->NumberValue();
  v8::Local<v8::Object> object = args[1]->ToObject();

  scoped_ptr<V8ValueConverter> converter(V8ValueConverter::create());
  scoped_ptr<base::Value> value(
      converter->FromV8Value(object, context()->v8_context()));

  if(!value) {
    return;                                                             
  }
  if(!value->IsType(base::Value::TYPE_DICTIONARY)) {
    return;
  }

  scoped_ptr<base::DictionaryValue> params(
      static_cast<base::DictionaryValue*>(value.release()));

  LOG(INFO) << "WEB_VIEW_BINDINGS: SetAutoSize " << guest_instance_id;

  render_frame_observer_->Send(
      new ThrustFrameHostMsg_WebViewGuestSetAutoSize(
        render_frame_observer_->routing_id(), 
        guest_instance_id, *params.get()));
}

void 
WebViewBindings::LoadUrl(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if(args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsString()) {
    NOTREACHED();
    return;
  }

  int guest_instance_id = args[0]->NumberValue();
  std::string url(*v8::String::Utf8Value(args[1]));

  LOG(INFO) << "WEB_VIEW_BINDINGS: LoadUrl " << guest_instance_id << " " << url;
  
  render_frame_observer_->Send(
      new ThrustFrameHostMsg_WebViewGuestLoadUrl(
        render_frame_observer_->routing_id(), 
        guest_instance_id, url));
}

void 
WebViewBindings::Go(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if(args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
    NOTREACHED();
    return;
  }

  int guest_instance_id = args[0]->NumberValue();
  int index = args[1]->NumberValue();

  LOG(INFO) << "WEB_VIEW_BINDINGS: Go " << guest_instance_id << " " << index;
  
  render_frame_observer_->Send(
      new ThrustFrameHostMsg_WebViewGuestGo(
        render_frame_observer_->routing_id(), 
        guest_instance_id, index));
}

void 
WebViewBindings::Reload(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if(args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsBoolean()) {
    NOTREACHED();
    return;
  }

  int guest_instance_id = args[0]->NumberValue();
  bool ignore_cache = args[1]->BooleanValue();

  LOG(INFO) << "WEB_VIEW_BINDINGS: Reload " << guest_instance_id << " " << ignore_cache;
  
  render_frame_observer_->Send(
      new ThrustFrameHostMsg_WebViewGuestReload(
        render_frame_observer_->routing_id(), 
        guest_instance_id, ignore_cache));
}


}  // namespace extensions
