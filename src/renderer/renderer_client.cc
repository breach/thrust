// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/renderer/renderer_client.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "v8/include/v8.h"
#include "third_party/WebKit/public/platform/WebMediaStreamCenter.h"
#include "third_party/WebKit/public/web/WebPluginParams.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "components/visitedlink/renderer/visitedlink_slave.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/public/test/layouttest_support.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/renderer/render_process_observer.h"
#include "exo_browser/src/renderer/render_view_observer.h"

using namespace content;

using blink::WebAudioDevice;
using blink::WebClipboard;
using blink::WebFrame;
using blink::WebMIDIAccessor;
using blink::WebMIDIAccessorClient;
using blink::WebMediaStreamCenter;
using blink::WebMediaStreamCenterClient;
using blink::WebPlugin;
using blink::WebPluginParams;
using blink::WebRTCPeerConnectionHandler;
using blink::WebRTCPeerConnectionHandlerClient;
using blink::WebThemeEngine;

namespace exo_browser {

namespace {
ExoBrowserRendererClient* g_renderer_client;
}

ExoBrowserRendererClient* 
ExoBrowserRendererClient::Get() 
{
  return g_renderer_client;
}

ExoBrowserRendererClient::ExoBrowserRendererClient() 
{
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

ExoBrowserRendererClient::~ExoBrowserRendererClient() 
{
  g_renderer_client = NULL;
}

void 
ExoBrowserRendererClient::RenderThreadStarted() 
{
  observer_.reset(new ExoBrowserRenderProcessObserver());
  visited_link_slave_.reset(new visitedlink::VisitedLinkSlave());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif

  RenderThread* thread = RenderThread::Get();
  thread->AddObserver(visited_link_slave_.get());
}

void 
ExoBrowserRendererClient::RenderViewCreated(
    RenderView* render_view) 
{
  new ExoBrowserRenderViewObserver(render_view);
}

bool 
ExoBrowserRendererClient::OverrideCreatePlugin(
    content::RenderFrame* render_frame,
    WebFrame* frame,
    const WebPluginParams& params,
    WebPlugin** plugin) {
  std::string mime_type = params.mimeType.utf8();
  if (mime_type == content::kBrowserPluginMimeType) {
    // Allow browser plugin in content_shell only if it is forced by flag.
    // Returning true here disables the plugin.
    return !CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kEnableBrowserPluginForAllViewTypes);
  }
  return false;
}

unsigned long long 
ExoBrowserRendererClient::VisitedLinkHash(
    const char* canonical_url, 
    size_t length) 
{ 
  return visited_link_slave_->ComputeURLFingerprint(canonical_url, length);
}

bool 
ExoBrowserRendererClient::IsLinkVisited(
    unsigned long long link_hash)
{
  //LOG(INFO) << link_hash << " " << visited_link_slave_->IsVisited(link_hash);
  return visited_link_slave_->IsVisited(link_hash);
}


} // namespace exo_browser
