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
  // ### WebViewBindings
  WebViewBindings(ScriptContext* context);

 private:
  // ### CreateGuest
  //
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void CreateGuest(const v8::FunctionCallbackInfo<v8::Value>& args);

  // ### DestroyGuest
  //
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void DestroyGuest(const v8::FunctionCallbackInfo<v8::Value>& args);

  // ### LoadUrl
  //
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void LoadUrl(const v8::FunctionCallbackInfo<v8::Value>& args);
  
  // ### Go
  //
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void Go(const v8::FunctionCallbackInfo<v8::Value>& args);

  // ### Reload
  //
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void Reload(const v8::FunctionCallbackInfo<v8::Value>& args);
};

}  // namespace extensions

#endif // THRUST_SHELL_RENDERER_EXTENSIONS_WEB_VIEW_BINDINGS_H_
