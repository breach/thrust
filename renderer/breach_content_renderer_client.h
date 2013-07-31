// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_RENDERER_BREACH_CONTENT_RENDERER_CLIENT_H_
#define BREACH_RENDERER_BREACH_CONTENT_RENDERER_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "base/platform_file.h"
#include "content/public/renderer/content_renderer_client.h"

namespace WebKit {
class WebFrame;
class WebPlugin;
struct WebPluginParams;
}

namespace breach {

class BreachRenderProcessObserver;

class BreachContentRendererClient : public content::ContentRendererClient {
 public:
  static BreachContentRendererClient* Get();

  BreachContentRendererClient();
  virtual ~BreachContentRendererClient();

  // ContentRendererClient implementation.
  virtual void RenderThreadStarted() OVERRIDE;
  virtual void RenderViewCreated(content::RenderView* render_view) OVERRIDE;

  virtual bool OverrideCreatePlugin(
      content::RenderView* render_view,
      WebKit::WebFrame* frame,
      const WebKit::WebPluginParams& params,
      WebKit::WebPlugin** plugin) OVERRIDE;

 private:
  scoped_ptr<BreachRenderProcessObserver> observer_;
};

}  // namespace content

#endif  // BREACH_RENDERER_BREACH_CONTENT_RENDERER_CLIENT_H_
