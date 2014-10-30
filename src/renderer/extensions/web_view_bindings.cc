// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2013 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/extensions/web_view_bindings.h"

#include <string>

#include "base/bind.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "v8/include/v8.h"

#include "src/renderer/extensions/script_context.h"

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
  if(args.Length() != 3 || 
     !args[0]->IsString() || !args[1]->IsObject() || !args[2]->IsFunction()) {
    NOTREACHED();
    return;
  }

  std::string type(*v8::String::Utf8Value(args[0]));
  v8::Local<v8::Object> params = args[1]->ToObject();
  LOG(INFO) << "WEB_VIEW_BINDINGS: CreateGuest " << type;

  //context()->CallFunction(v8::Handle<v8::Function>::Cast(args[0]), 0, &no_args);

  /* TODO(spolu): call CreateGuest */
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
