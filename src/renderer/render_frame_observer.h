// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_RENDER_FRAME_OBSERVER_H_
#define THRUST_SHELL_RENDERER_RENDER_FRAME_OBSERVER_H_

#include <vector>

#include "base/values.h"

#include "content/public/renderer/render_frame_observer.h"

namespace blink {
class WebFrame;
}

namespace content {
class RenderFrame;
}

namespace thrust_shell {

class ThrustShellRenderFrameObserver : public content::RenderFrameObserver {
 public:
  explicit ThrustShellRenderFrameObserver(content::RenderFrame* render_frame);
  virtual ~ThrustShellRenderFrameObserver();

  static ThrustShellRenderFrameObserver* 
    FromRenderFrame(content::RenderFrame* render_frame);

  /****************************************************************************/
  /* RENDERFRAMEOBSERVER IMPLEMENTATION */
  /****************************************************************************/
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  /****************************************************************************/
  /* WEBVIEW MESSAGE HANDLING */
  /****************************************************************************/
  void WebViewGuestEmit(int guest_instance_id,
                        const std::string type,
                        const base::DictionaryValue& params);

 private:
  // A static container of all the instances.
  static std::vector<ThrustShellRenderFrameObserver*> s_instances;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellRenderFrameObserver);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_RENDERER_RENDER_FRAME_OBSERVER_H_
