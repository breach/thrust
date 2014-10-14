// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_EXTENSIONS_DISPATCHER_H_
#define THRUST_SHELL_RENDERER_EXTENSIONS_DISPATCHER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "ipc/ipc_platform_file.h"
#include "content/public/renderer/render_process_observer.h"

#include "src/renderer/extensions/local_source_map.h"

namespace blink {
class WebFrame;
class WebSecurityOrigin;
}

namespace base {
class DictionaryValue;
class ListValue;
}

namespace content {
class RenderThread;
}

namespace extensions {

class LocalSourceMap;
class ModuleSystem;
class ScriptContext;

// ### Dispatcher
//
// Dispatches extension control messages sent to the renderer and stores
// renderer extension related state.
class Dispatcher : public content::RenderProcessObserver {
public:
  Dispatcher();
  virtual ~Dispatcher();

  void DidCreateScriptContext(blink::WebFrame* frame,
                              v8::Handle<v8::Context> context,
                              int extension_group,
                              int world_id);
  void WillReleaseScriptContext(blink::WebFrame* frame,
                                v8::Handle<v8::Context> context,
                                int world_id);
  void DidCreateDocumentElement(blink::WebFrame* frame);

  /****************************************************************************/
  /* RENDERPROCESSOBSERVER API */
  /****************************************************************************/
  virtual void WebKitInitialized() OVERRIDE;
  virtual void IdleNotification() OVERRIDE;
  virtual void OnRenderProcessShutdown() OVERRIDE;

private:
  /****************************************************************************/
  /* INTERNAL API */
  /****************************************************************************/
  void PopulateSourceMap();
  void EnableCustomElementWhiteList();
  void RegisterNativeHandlers(ModuleSystem* module_system,
                              ScriptContext* context);

  bool                    is_webkit_initialized_;
  LocalSourceMap          source_map_;

  DISALLOW_COPY_AND_ASSIGN(Dispatcher);
};

} // namespace extensions

#endif // THRUST_SHELL_RENDERER_EXTENSIONS_DISPATCHER_H_
