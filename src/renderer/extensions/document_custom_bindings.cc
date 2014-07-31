// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/renderer/extensions/document_custom_bindings.h"

#include <string>

#include "base/bind.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "v8/include/v8.h"

#include "src/renderer/extensions/context.h"

namespace extensions {

DocumentCustomBindings::DocumentCustomBindings(
    Context* context)
  : ObjectBackedNativeHandler(context) 
{
  RouteFunction("RegisterElement",
      base::Bind(&DocumentCustomBindings::RegisterElement,
                 base::Unretained(this)));
}

// Attach an event name to an object.
void DocumentCustomBindings::RegisterElement(
    const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  if (args.Length() != 2 || !args[0]->IsString() || !args[1]->IsObject()) {
    NOTREACHED();
    return;
  }

  std::string element_name(*v8::String::Utf8Value(args[0]));
  v8::Local<v8::Object> options = args[1]->ToObject();

  blink::WebExceptionCode ec = 0;
  blink::WebDocument document = context()->web_frame()->document();
  v8::Handle<v8::Value> constructor =
      document.registerEmbedderCustomElement(
          blink::WebString::fromUTF8(element_name), options, ec);
  args.GetReturnValue().Set(constructor);
}

}  // namespace extensions
