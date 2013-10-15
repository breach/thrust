// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/renderer/content_renderer_client.h"

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

using WebKit::WebAudioDevice;
using WebKit::WebClipboard;
using WebKit::WebFrame;
using WebKit::WebMIDIAccessor;
using WebKit::WebMIDIAccessorClient;
using WebKit::WebMediaStreamCenter;
using WebKit::WebMediaStreamCenterClient;
using WebKit::WebPlugin;
using WebKit::WebPluginParams;
using WebKit::WebRTCPeerConnectionHandler;
using WebKit::WebRTCPeerConnectionHandlerClient;
using WebKit::WebThemeEngine;

namespace exo_browser {

namespace {
ExoBrowserContentRendererClient* g_renderer_client;
}

ExoBrowserContentRendererClient* 
ExoBrowserContentRendererClient::Get() 
{
  return g_renderer_client;
}

ExoBrowserContentRendererClient::ExoBrowserContentRendererClient() 
{
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

ExoBrowserContentRendererClient::~ExoBrowserContentRendererClient() 
{
  g_renderer_client = NULL;
}

void 
ExoBrowserContentRendererClient::RenderThreadStarted() 
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
ExoBrowserContentRendererClient::RenderViewCreated(
    RenderView* render_view) 
{
  new ExoBrowserRenderViewObserver(render_view);
}

bool 
ExoBrowserContentRendererClient::OverrideCreatePlugin(
    RenderView* render_view,
    WebFrame* frame,
    const WebPluginParams& params,
    WebPlugin** plugin) 
{
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
ExoBrowserContentRendererClient::VisitedLinkHash(
    const char* canonical_url, 
    size_t length) 
{ 
  return visited_link_slave_->ComputeURLFingerprint(canonical_url, length);
}

bool 
ExoBrowserContentRendererClient::IsLinkVisited(
    unsigned long long link_hash)
{
  //LOG(INFO) << link_hash << " " << visited_link_slave_->IsVisited(link_hash);
  return visited_link_slave_->IsVisited(link_hash);
}


} // namespace exo_browser
