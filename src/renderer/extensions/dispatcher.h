// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_RENDERER_EXTENSIONS_DISPATCHER_H_
#define EXO_SHELL_RENDERER_EXTENSIONS_DISPATCHER_H_

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
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void WebKitInitialized() OVERRIDE;
  virtual void IdleNotification() OVERRIDE;
  virtual void OnRenderProcessShutdown() OVERRIDE;

private:
  /****************************************************************************/
  /* INTERNAL API */
  /****************************************************************************/
  void EnableCustomElementWhiteList();

  bool                    is_webkit_initialized_;
  LocalSourceMap          source_map_;

  DISALLOW_COPY_AND_ASSIGN(Dispatcher);
};

}  // namespace extensions

#endif EXO_SHELL_RENDERER_EXTENSIONS_DISPATCHER_H_
