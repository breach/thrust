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
#include "src/renderer/extensions/web_view_bindings.h"
#include "src/renderer/extensions/remote_bindings.h"

using namespace content;

namespace thrust_shell {

/******************************************************************************/
/* STATIC API */
/******************************************************************************/
std::vector<ThrustShellRenderFrameObserver*> 
  ThrustShellRenderFrameObserver::s_instances;

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

/******************************************************************************/
/* CONSTRUCTOR / DESTRUCTOR */
/******************************************************************************/
ThrustShellRenderFrameObserver::ThrustShellRenderFrameObserver(
    RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) 
{
  LOG(INFO) << "RENDER FRAME CREATED " << render_frame;
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


/******************************************************************************/
/* RENDERFRAMEOBSERVER IMPLEMENTATION */
/******************************************************************************/
bool 
ThrustShellRenderFrameObserver::OnMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ThrustShellRenderFrameObserver, message)
    IPC_MESSAGE_HANDLER(ThrustFrameMsg_WebViewEmit, 
                        WebViewEmit)
    IPC_MESSAGE_HANDLER(ThrustFrameMsg_RemoteDispatch, 
                        RemoteDispatch)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

/******************************************************************************/
/* WEBVIEW MESSAGE HANDLING */
/******************************************************************************/
void 
ThrustShellRenderFrameObserver::AddWebViewBindings(
    extensions::WebViewBindings* bindings)
{
  /* Does not own bindings just stores a pointer to it. */
  web_view_bindings_.push_back(bindings);
}

void 
ThrustShellRenderFrameObserver::RemoveWebViewBindings(
    extensions::WebViewBindings* bindings)
{
  for(size_t i = 0; i < web_view_bindings_.size(); ++i) {
    if(web_view_bindings_[i] == bindings) {
      web_view_bindings_.erase(web_view_bindings_.begin() + i);
      break;
    }
  }
}

void 
ThrustShellRenderFrameObserver::WebViewEmit(
    int guest_instance_id,
    const std::string type,
    const base::DictionaryValue& event)
{
  for(size_t i = 0; i < web_view_bindings_.size(); ++i) {
    web_view_bindings_[i]->AttemptEmitEvent(guest_instance_id, type, event);
  }
}

/******************************************************************************/
/* REMOTE MESSAGE HANDLING */
/******************************************************************************/
void 
ThrustShellRenderFrameObserver::AddRemoteBindings(
    extensions::RemoteBindings* bindings)
{
  /* Does not own bindings just stores a pointer to it. */
  remote_bindings_.push_back(bindings);
}

void 
ThrustShellRenderFrameObserver::RemoveRemoteBindings(
    extensions::RemoteBindings* bindings)
{
  for(size_t i = 0; i < remote_bindings_.size(); ++i) {
    if(remote_bindings_[i] == bindings) {
      remote_bindings_.erase(remote_bindings_.begin() + i);
      break;
    }
  }
}


void
ThrustShellRenderFrameObserver::RemoteDispatch(
    const base::DictionaryValue& message)
{
  for(size_t i = 0; i < remote_bindings_.size(); ++i) {
    /* TODO(spolu): THERE SHOULD BE ONLY ONE */
    remote_bindings_[i]->DispatchMessage(message);
  }
}


}  // namespace thrust_shell
