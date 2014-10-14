// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_EXTENSIONS_CONTEXT_H_
#define THRUST_SHELL_RENDERER_EXTENSIONS_CONTEXT_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "url/gurl.h"
#include "v8/include/v8.h"

#include "src/renderer/extensions/module_system.h"
#include "src/renderer/extensions/safe_builtins.h"
#include "src/renderer/extensions/scoped_persistent.h"

namespace blink {
class WebFrame;
class WebDocument;
}

namespace content {
class RenderView;
}

namespace extensions {

// Chrome's wrapper for a v8 context.
class Context {
 public:
   Context(v8::Handle<v8::Context> context,
           blink::WebFrame* frame);
  virtual ~Context();

  // Clears the WebFrame for this contexts and invalidates the associated
  // ModuleSystem.
  void Invalidate();

  // Returns true if this context is still valid, false if it isn't.
  // A context becomes invalid via Invalidate().
  bool is_valid() const {
    return !v8_context_.IsEmpty();
  }

  v8::Handle<v8::Context> v8_context() const {
    return v8_context_.NewHandle(v8::Isolate::GetCurrent());
  }

  blink::WebFrame* web_frame() const {
    return web_frame_;
  }

  void set_module_system(scoped_ptr<ModuleSystem> module_system) {
    module_system_ = module_system.Pass();
  }

  ModuleSystem* module_system() { return module_system_.get(); }

  SafeBuiltins* safe_builtins() {
    return &safe_builtins_;
  }
  const SafeBuiltins* safe_builtins() const {
    return &safe_builtins_;
  }

  // Returns the RenderView associated with this context. Can return NULL if the
  // context is in the process of being destroyed.
  content::RenderView* GetRenderView() const;

  // Get the URL of this context's web frame.
  GURL GetURL() const;

  // Runs |function| with appropriate scopes. Doesn't catch exceptions, callers
  // must do that if they want.
  //
  // USE THIS METHOD RATHER THAN v8::Function::Call WHEREVER POSSIBLE.
  v8::Local<v8::Value> CallFunction(v8::Handle<v8::Function> function,
                                    int argc,
                                    v8::Handle<v8::Value> argv[]) const;

  v8::Isolate* isolate() const {
    return isolate_;
  }

 private:
  // The v8 context the bindings are accessible to.
  ScopedPersistent<v8::Context> v8_context_;

  // The WebFrame associated with this context. This can be NULL because this
  // object can outlive is destroyed asynchronously.
  blink::WebFrame* web_frame_;

  // Owns and structures the JS that is injected to set up extension bindings.
  scoped_ptr<ModuleSystem> module_system_;

  // Contains safe copies of builtin objects like Function.prototype.
  SafeBuiltins safe_builtins_;

  v8::Isolate* isolate_;

  DISALLOW_COPY_AND_ASSIGN(Context);
};

}  // namespace extensions

#endif // THRUST_SHELL_RENDERER_EXTENSIONS_CONTEXT_H_
