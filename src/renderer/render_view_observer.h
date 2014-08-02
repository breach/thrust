// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_RENDERER_RENDER_VIEW_OBSERVER_H_
#define EXO_SHELL_RENDERER_RENDER_VIEW_OBSERVER_H_

#include "content/public/renderer/render_view_observer.h"

namespace blink {
class WebFrame;
}

namespace content {
class RenderView;
}

namespace exo_shell {

class ExoShellRenderViewObserver : public content::RenderViewObserver {
 public:
  explicit ExoShellRenderViewObserver(content::RenderView* render_view);
  virtual ~ExoShellRenderViewObserver() {}

 private:
  /****************************************************************************/
  /* RENDERVIEWOBSERVER IMPLEMENTATION                                        */
  /****************************************************************************/
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void DidClearWindowObject(blink::WebFrame* frame,
                                    int world_id) OVERRIDE;
  virtual void DraggableRegionsChanged(blink::WebFrame* frame) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(ExoShellRenderViewObserver);
};

} // namespace exo_shell

#endif // EXO_SHELL_RENDERER_RENDER_VIEW_OBSERVER_H_
