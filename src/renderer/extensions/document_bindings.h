// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2013 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_EXTENSIONS_DOCUMENT_BINDINGS_H_
#define THRUST_SHELL_RENDERER_EXTENSIONS_DOCUMENT_BINDINGS_H_

#include "src/renderer/extensions/object_backed_native_handler.h"

namespace extensions {

class ScriptContext;

class DocumentBindings : public ObjectBackedNativeHandler {
 public:
  // ### DocumentBindings
  DocumentBindings(ScriptContext* context);

 private:
  // ### RegisterElement
  //
  // Registers the provided element as a custom element in Blink.
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void RegisterElement(const v8::FunctionCallbackInfo<v8::Value>& args);
};

}  // namespace extensions

#endif // THRUST_SHELL_RENDERER_EXTENSIONS_DOCUMENT_BINDINGS_H_
