// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_CONTENT_RENDERER_CLIENT_H_
#define THRUST_SHELL_RENDERER_CONTENT_RENDERER_CLIENT_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/content_renderer_client.h"

#include "src/renderer/extensions/local_source_map.h"

namespace visitedlink {                                                         
class VisitedLinkSlave;                                                         
}    

namespace blink {
class WebLocalFrame;
class WebFrame;
class WebPlugin;
class WebPluginContainer;
struct WebPluginParams;
}

namespace base {
class DictionaryValue;
class ListValue;
}

namespace thrust_shell {

class ThrustShellRenderProcessObserver;
class Dispatcher;

class ThrustShellRendererClient : public content::ContentRendererClient {
 public:
  static ThrustShellRendererClient* Get();

  ThrustShellRendererClient();
  virtual ~ThrustShellRendererClient();

  /****************************************************************************/
  /* CONTENTRENDERERCLIENT IMPLEMENTATION */
  /****************************************************************************/
  virtual void RenderThreadStarted() OVERRIDE;
  virtual void RenderViewCreated(content::RenderView* render_view) OVERRIDE;
  virtual void RenderFrameCreated(content::RenderFrame* render_frame) OVERRIDE;

  virtual bool OverrideCreatePlugin(
      content::RenderFrame* render_frame,
      blink::WebLocalFrame* frame,
      const blink::WebPluginParams& params,
      blink::WebPlugin** plugin) OVERRIDE;

  virtual void DidCreateScriptContext(
      blink::WebFrame* frame, 
      v8::Handle<v8::Context> context, 
      int extension_group,
      int world_id) OVERRIDE;
  bool ShouldFork(blink::WebFrame* frame,
                  const GURL& url,
                  const std::string& http_method,
                  bool is_initial_navigation,
                  bool is_server_redirect,
                  bool* send_referrer) OVERRIDE;

  virtual unsigned long long VisitedLinkHash(const char* canonical_url,         
                                             size_t length) OVERRIDE;           
  virtual bool IsLinkVisited(unsigned long long link_hash) OVERRIDE;   


 private:
  scoped_ptr<ThrustShellRenderProcessObserver> observer_;
  scoped_ptr<visitedlink::VisitedLinkSlave>    visited_link_slave_;
  extensions::LocalSourceMap                   source_map_;
};

} // namespace thrust_shell

#endif // THRUST_SHELL_RENDERER_CONTENT_RENDERER_CLIENT_H_
