// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_SCRIPT_CONTEXT_H_
#define EXTENSIONS_RENDERER_SCRIPT_CONTEXT_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "v8/include/v8.h"

#include "src/renderer/extensions/module_system.h"
#include "src/renderer/extensions/safe_builtins.h"
#include "src/renderer/extensions/scoped_persistent.h"

class GURL;

namespace blink {
class WebFrame;
}

namespace content {
class RenderView;
}

namespace extensions {

// Wrapper for a v8 context.
class ScriptContext {
 public:
  ScriptContext(const v8::Handle<v8::Context>& context,
                blink::WebFrame* frame);
  virtual ~ScriptContext();

  // Clears the WebFrame for this contexts and invalidates the associated
  // ModuleSystem.
  void Invalidate();

  // Returns true if this context is still valid, false if it isn't.
  // A context becomes invalid via Invalidate().
  bool is_valid() const { 
    return !v8_context_.IsEmpty(); 
  }

  v8::Handle<v8::Context> v8_context() const {
    return v8_context_.NewHandle(isolate());
  }

  blink::WebFrame* web_frame() const { 
    return web_frame_; 
  }

  void set_module_system(scoped_ptr<ModuleSystem> module_system) {
    module_system_ = module_system.Pass();
  }

  ModuleSystem* module_system() { return module_system_.get(); }

  SafeBuiltins* safe_builtins() { return &safe_builtins_; }

  const SafeBuiltins* safe_builtins() const { return &safe_builtins_; }

  // Returns the RenderView associated with this context. Can return NULL if the
  // context is in the process of being destroyed.
  content::RenderView* GetRenderView() const;

  // Runs |function| with appropriate scopes. Doesn't catch exceptions, callers
  // must do that if they want.
  //
  // USE THIS METHOD RATHER THAN v8::Function::Call WHEREVER POSSIBLE.
  v8::Local<v8::Value> CallFunction(v8::Handle<v8::Function> function,
                                    int argc,
                                    v8::Handle<v8::Value> argv[]) const;

  // Fires the onunload event on the unload_event module.
  void DispatchOnUnloadEvent();

  v8::Isolate* isolate() const { return isolate_; }

  // Get the URL of this context's web frame.
  GURL GetURL() const;

  // Utility to get the URL we will match against for a frame. If the frame has
  // committed, this is the commited URL. Otherwise it is the provisional URL.
  static GURL GetDataSourceURLForFrame(const blink::WebFrame* frame);

  // Returns the first non-about:-URL in the document hierarchy above and
  // including |frame|. The document hierarchy is only traversed if
  // |document_url| is an about:-URL and if |match_about_blank| is true.
  static GURL GetEffectiveDocumentURL(const blink::WebFrame* frame,
                                      const GURL& document_url,
                                      bool match_about_blank);


 protected:
  // The v8 context the bindings are accessible to.
  ScopedPersistent<v8::Context> v8_context_;

 private:
  // The WebFrame associated with this context. This can be NULL because this
  // object can outlive is destroyed asynchronously.
  blink::WebFrame* web_frame_;

  // Owns and structures the JS that is injected to set up extension bindings.
  scoped_ptr<ModuleSystem> module_system_;

  // Contains safe copies of builtin objects like Function.prototype.
  SafeBuiltins safe_builtins_;

  v8::Isolate* isolate_;

  DISALLOW_COPY_AND_ASSIGN(ScriptContext);
};

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_SCRIPT_CONTEXT_H_
