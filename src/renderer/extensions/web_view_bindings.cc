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
  RouteFunction("RegisterElement",
      base::Bind(&WebViewBindings::RegisterElement,
                 base::Unretained(this)));
}

// Attach an event name to an object.
void WebViewBindings::RegisterElement(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {
    NOTREACHED();
    return;
  }

  std::string element_name(*v8::String::Utf8Value(args[0]));
  LOG(INFO) << "CUSTOM BINDING: " << element_name;
  v8::Local<v8::Object> options = args[1]->ToObject();

  blink::WebExceptionCode ec = 0;
  blink::WebDocument document = context()->web_frame()->document();

  v8::Handle<v8::Value> constructor =
      document.registerEmbedderCustomElement(
          blink::WebString::fromUTF8(element_name), options, ec);
  args.GetReturnValue().Set(constructor);
}

}  // namespace extensions
