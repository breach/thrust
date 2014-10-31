// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/renderer/render_frame_observer.h"

#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebDraggableRegion.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_frame_observer.h"

#include "src/common/switches.h"
#include "src/common/messages.h"

using namespace content;

namespace thrust_shell {

std::vector<ThrustShellRenderFrameObserver*> 
  ThrustShellRenderFrameObserver::s_instances;

ThrustShellRenderFrameObserver::ThrustShellRenderFrameObserver(
    RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) 
{
  LOG(INFO) << "************* RENDER FRAME CREATED " << render_frame;
  s_instances.push_back(this);
}

ThrustShellRenderFrameObserver::~ThrustShellRenderFrameObserver()
{
  for(size_t i = 0; i < s_instances.size(); ++i) {
    if(s_instances[i] == this) {
      s_instances.erase(s_instances.begin() + i);
      break;
    }
  }
}

// static
ThrustShellRenderFrameObserver* 
ThrustShellRenderFrameObserver::FromRenderFrame(
    RenderFrame* render_frame)
{
  for(size_t i = 0; i < s_instances.size(); ++i) {
    if(s_instances[i]->render_frame() == render_frame) {
      return s_instances[i];
    }
  }
  return NULL;
}

bool 
ThrustShellRenderFrameObserver::OnMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ThrustShellRenderFrameObserver, message)
    IPC_MESSAGE_HANDLER(ThrustFrameMsg_WebViewGuestEmit, 
                        WebViewGuestEmit)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void 
ThrustShellRenderFrameObserver::WebViewGuestEmit(
    int guest_instance_id,
    const std::string type,
    const base::DictionaryValue& params)
{
  LOG(INFO) << "++++++++++++++ EVENT : " << guest_instance_id << " " << type;
}

}  // namespace thrust_shell
