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
  RouteFunction("LoadUrl",
      base::Bind(&WebViewBindings::LoadUrl,
                 base::Unretained(this)));
  RouteFunction("Go",
      base::Bind(&WebViewBindings::Go,
                 base::Unretained(this)));
  RouteFunction("Reload",
      base::Bind(&WebViewBindings::Reload,
                 base::Unretained(this)));
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
  LOG(INFO) << "WEB_VIEW_BINDINGS: CreateGuest";

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

  thrust_shell::ThrustShellRenderFrameObserver* render_frame_observer = 
    thrust_shell::ThrustShellRenderFrameObserver::FromRenderFrame(
        RenderFrame::FromWebFrame(context()->web_frame()));

  int id = 0;
  render_frame_observer->Send(
      new ThrustFrameHostMsg_CreateWebViewGuest(
        render_frame_observer->routing_id(), 
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
    //v8::Number::Value(args[0]);
  LOG(INFO) << "WEB_VIEW_BINDINGS: DestroyGuest " << guest_instance_id;
  
  /* TODO(spolu): call DestroyGuest */
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
  
  /* TODO(spolu): call LoadUrl */
  thrust_shell::ThrustShellRenderFrameObserver* render_frame_observer = 
    thrust_shell::ThrustShellRenderFrameObserver::FromRenderFrame(
        RenderFrame::FromWebFrame(context()->web_frame()));

  render_frame_observer->Send(
      new ThrustFrameHostMsg_WebViewGuestLoadUrl(
        render_frame_observer->routing_id(), 
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
  
  /* TODO(spolu): call Go */
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
  
  /* TODO(spolu): call Reload */
}


}  // namespace extensions
