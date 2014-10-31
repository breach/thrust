// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/render_process_observer.h"

#include "base/command_line.h"
#include "content/public/common/content_client.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/public/test/layouttest_support.h"
#include "third_party/WebKit/public/web/WebCustomElement.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebPluginParams.h"
#include "third_party/WebKit/public/web/WebKit.h"
#include "third_party/WebKit/public/web/WebRuntimeFeatures.h"

#include "src/common/messages.h"
#include "src/common/switches.h"
#include "src/renderer/renderer_client.h"

using namespace content;

namespace thrust_shell {

namespace {

ThrustShellRenderProcessObserver* g_instance = NULL;

bool 
IsSwitchEnabled(
    base::CommandLine* command_line,
    const char* switch_string,
    bool* enabled) 
{
  std::string value = command_line->GetSwitchValueASCII(switch_string);
  if (value == "true")
    *enabled = true;
  else if (value == "false")
    *enabled = false;
  else
    return false;
  return true;
}
 
}

// static
ThrustShellRenderProcessObserver* 
ThrustShellRenderProcessObserver::GetInstance() 
{
  return g_instance;
}

ThrustShellRenderProcessObserver::ThrustShellRenderProcessObserver()
{
  CHECK(!g_instance);
  g_instance = this;
}

ThrustShellRenderProcessObserver::~ThrustShellRenderProcessObserver() 
{
  CHECK(g_instance == this);
  g_instance = NULL;
}

void 
ThrustShellRenderProcessObserver::WebKitInitialized() 
{
  EnableWebRuntimeFeatures();
  blink::WebCustomElement::addEmbedderCustomElementName("browserplugin"); 
  blink::WebCustomElement::addEmbedderCustomElementName("webview");
}

bool 
ThrustShellRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  /*
  IPC_BEGIN_MESSAGE_MAP(ThrustShellRenderProcessObserver, message)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  */
  handled = false;

  return handled;
}

void 
ThrustShellRenderProcessObserver::EnableWebRuntimeFeatures() 
{
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  bool b;
  if(IsSwitchEnabled(command_line, switches::kExperimentalFeatures, &b)) {
    blink::WebRuntimeFeatures::enableExperimentalFeatures(b);
  }
  if(IsSwitchEnabled(command_line, switches::kExperimentalCanvasFeatures, &b)) {
    blink::WebRuntimeFeatures::enableExperimentalCanvasFeatures(b);
  }
  if(IsSwitchEnabled(command_line, switches::kSubpixelFontScaling, &b)) {
    blink::WebRuntimeFeatures::enableSubpixelFontScaling(b);
  }
  if(IsSwitchEnabled(command_line, switches::kOverlayScrollbars, &b)) {
    blink::WebRuntimeFeatures::enableOverlayScrollbars(b);
  }
  if(IsSwitchEnabled(command_line, switches::kOverlayFullscreenVideo, &b)) {
    blink::WebRuntimeFeatures::enableOverlayFullscreenVideo(b);
  }
  if(IsSwitchEnabled(command_line, switches::kSharedWorker, &b)) {
    blink::WebRuntimeFeatures::enableSharedWorker(b);
  }
}


} // namespace thrust_shell
