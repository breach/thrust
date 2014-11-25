// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_EXTENSIONS_REMOTE_BINDINGS_H_
#define THRUST_SHELL_RENDERER_EXTENSIONS_REMOTE_BINDINGS_H_

#include "base/values.h"

#include "src/renderer/extensions/object_backed_native_handler.h"

namespace thrust_shell {
class ThrustShellRenderFrameObserver;
}

namespace extensions {

class ScriptContext;

class RemoteBindings : public ObjectBackedNativeHandler {
 public:
  // ### RemoteBindings
  RemoteBindings(ScriptContext* context);
  ~RemoteBindings();

  // ### DispatchMessage
  //
  // Sends a message to the remote object to be dispatched any listener
  bool DispatchMessage(const base::DictionaryValue& message);

 private:

  // ### [RouteFunction]
  //
  // ```
  // @args {FunctionCallbackInfo} v8 args and return
  // ```
  void SendMessage(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SetHandler(const v8::FunctionCallbackInfo<v8::Value>& args);

  v8::Persistent<v8::Function>                          message_handler_;
  thrust_shell::ThrustShellRenderFrameObserver*         render_frame_observer_;
};

}  // namespace extensions

#endif // THRUST_SHELL_RENDERER_EXTENSIONS_REMOTE_BINDINGS_H_
