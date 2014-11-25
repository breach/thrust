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

namespace extensions {
class WebViewBindings;
class RemoteBindings;
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
  void AddWebViewBindings(extensions::WebViewBindings* bindings);
  void RemoveWebViewBindings(extensions::WebViewBindings* bindings);

  void WebViewEmit(int guest_instance_id,
                   const std::string type,
                   const base::DictionaryValue& event);

  /****************************************************************************/
  /* REMOTE MESSAGE HANDLING */
  /****************************************************************************/
  void AddRemoteBindings(extensions::RemoteBindings* bindings);
  void RemoveRemoteBindings(extensions::RemoteBindings* bindings);

  void RemoteDispatch(const base::DictionaryValue& message);

 private:
  // A static container of all the instances.
  static std::vector<ThrustShellRenderFrameObserver*> s_instances;

  std::vector<extensions::WebViewBindings*>           web_view_bindings_;
  std::vector<extensions::RemoteBindings*>            remote_bindings_;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellRenderFrameObserver);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_RENDERER_RENDER_FRAME_OBSERVER_H_
