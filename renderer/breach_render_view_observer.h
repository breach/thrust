// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_RENDERER_BREACH_RENDER_VIEW_OBSERVER_H_
#define BREACH_RENDERER_BREACH_RENDER_VIEW_OBSERVER_H_

#include "content/public/renderer/render_view_observer.h"

namespace WebKit {
class WebFrame;
}

namespace content {
class RenderView;
}

namespace breach {

class BreachRenderViewObserver : public content::RenderViewObserver {
 public:
  explicit BreachRenderViewObserver(content::RenderView* render_view);
  virtual ~BreachRenderViewObserver() {}

 private:
  // RenderViewObserver implementation.
  virtual void DidClearWindowObject(WebKit::WebFrame* frame) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(BreachRenderViewObserver);
};

} // namespace breach

#endif // BREACH_RENDERER_BREACH_RENDER_VIEW_OBSERVER_H_
