// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_RENDER_VIEW_OBSERVER_H_
#define THRUST_SHELL_RENDERER_RENDER_VIEW_OBSERVER_H_

#include "content/public/renderer/render_view_observer.h"

namespace blink {
class WebFrame;
}

namespace content {
class RenderView;
}

namespace thrust_shell {

class ThrustShellRenderViewObserver : public content::RenderViewObserver {
 public:
  explicit ThrustShellRenderViewObserver(content::RenderView* render_view);
  virtual ~ThrustShellRenderViewObserver() {}

 private:
  /****************************************************************************/
  /* RENDERVIEWOBSERVER IMPLEMENTATION                                        */
  /****************************************************************************/
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void DraggableRegionsChanged(blink::WebFrame* frame) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellRenderViewObserver);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_RENDERER_RENDER_VIEW_OBSERVER_H_
