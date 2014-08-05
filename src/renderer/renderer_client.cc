// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/renderer_client.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "v8/include/v8.h"
#include "third_party/WebKit/public/platform/WebURL.h"
#include "third_party/WebKit/public/platform/WebURLError.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "third_party/WebKit/public/platform/WebMediaStreamCenter.h"
#include "third_party/WebKit/public/web/WebPluginParams.h"
#include "third_party/WebKit/public/web/WebElement.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/WebKit/public/web/WebPluginContainer.h"
#include "src/renderer/visitedlink/visitedlink_slave.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/public/test/layouttest_support.h"

#include "src/common/switches.h"
#include "src/renderer/render_process_observer.h"
#include "src/renderer/render_view_observer.h"
#include "src/renderer/extensions/dispatcher.h"

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

namespace exo_shell {

namespace {

ExoShellRendererClient* g_renderer_client;

const char kWebViewTagName[] = "WEBVIEW";

}

ExoShellRendererClient* 
ExoShellRendererClient::Get() 
{
  return g_renderer_client;
}

ExoShellRendererClient::ExoShellRendererClient() 
{
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

ExoShellRendererClient::~ExoShellRendererClient() 
{
  g_renderer_client = NULL;
}

void 
ExoShellRendererClient::RenderThreadStarted() 
{
  observer_.reset(new ExoShellRenderProcessObserver());
  visited_link_slave_.reset(new visitedlink::VisitedLinkSlave());
  if (!extension_dispatcher_)
    extension_dispatcher_.reset(new extensions::Dispatcher());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif

  RenderThread* thread = RenderThread::Get();
  thread->AddObserver(visited_link_slave_.get());
  thread->AddObserver(extension_dispatcher_.get());
}

void 
ExoShellRendererClient::RenderViewCreated(
    RenderView* render_view) 
{
  new ExoShellRenderViewObserver(render_view);
}

bool 
ExoShellRendererClient::OverrideCreatePlugin(
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

bool 
ExoShellRendererClient::AllowBrowserPlugin(
    blink::WebPluginContainer* container) 
{
  if (CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kEnableBrowserPluginForAllViewTypes))
    return true;

  // If this |BrowserPlugin| <object> in the |container| is not inside a
  // <webview>/<adview> shadowHost, we disable instantiating this plugin. This
  // is to discourage and prevent developers from accidentally attaching
  // <object> directly in apps.
  //
  // Note that this check below does *not* ensure any security, it is still
  // possible to bypass this check.
  // TODO(lazyboy): http://crbug.com/178663, Ensure we properly disallow
  // instantiating BrowserPlugin outside of the <webview>/<adview> shim.
  if (container->element().isNull())
    return false;

  if (container->element().shadowHost().isNull())
    return false;

  blink::WebString tag_name = container->element().shadowHost().tagName();
  return tag_name.equals(blink::WebString::fromUTF8(kWebViewTagName));
}

void 
ExoShellRendererClient::DidCreateScriptContext(
    blink::WebFrame* frame, 
    v8::Handle<v8::Context> context, 
    int extension_group,
    int world_id) {
  extension_dispatcher_->DidCreateScriptContext(frame, context, 
                                                extension_group, 
                                                world_id);
}

unsigned long long 
ExoShellRendererClient::VisitedLinkHash(
    const char* canonical_url, 
    size_t length) 
{ 
  return visited_link_slave_->ComputeURLFingerprint(canonical_url, length);
}

bool 
ExoShellRendererClient::IsLinkVisited(
    unsigned long long link_hash)
{
  //LOG(INFO) << link_hash << " " << visited_link_slave_->IsVisited(link_hash);
  return visited_link_slave_->IsVisited(link_hash);
}


} // namespace exo_shell
