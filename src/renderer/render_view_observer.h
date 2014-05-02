// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_RENDERER_RENDER_VIEW_OBSERVER_H_
#define EXO_BROWSER_RENDERER_RENDER_VIEW_OBSERVER_H_

#include "content/public/renderer/render_view_observer.h"

namespace blink {
class WebFrame;
}

namespace content {
class RenderView;
}

namespace exo_browser {

class ExoBrowserRenderViewObserver : public content::RenderViewObserver {
 public:
  explicit ExoBrowserRenderViewObserver(content::RenderView* render_view);
  virtual ~ExoBrowserRenderViewObserver() {}

 private:
  /****************************************************************************/
  /* RENDERVIEWOBSERVER IMPLEMENTATION                                        */
  /****************************************************************************/
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void DraggableRegionsChanged(blink::WebFrame* frame) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserRenderViewObserver);
};

} // namespace exo_browser

#endif // EXO_BROWSER_RENDERER_RENDER_VIEW_OBSERVER_H_
