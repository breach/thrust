// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_EXTENSIONS_WEB_VIEW_BINDINGS_H_
#define THRUST_SHELL_RENDERER_EXTENSIONS_WEB_VIEW_BINDINGS_H_

#include "src/renderer/extensions/object_backed_native_handler.h"

namespace extensions {
class ScriptContext;

class WebViewBindings : public ObjectBackedNativeHandler {
 public:
  WebViewBindings(ScriptContext* context);

 private:
  // Registers the provided element as a custom element in Blink.
  //void RegisterElement(const v8::FunctionCallbackInfo<v8::Value>& args);
  
  // ### CreateGuest
  //
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void CreateGuest(const v8::FunctionCallbackInfo<v8::Value>& args);
};

}  // namespace extensions

#endif // THRUST_SHELL_RENDERER_EXTENSIONS_WEB_VIEW_BINDINGS_H_
