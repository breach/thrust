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
#include "third_party/WebKit/public/web/WebLocalFrame.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "src/renderer/visitedlink/visitedlink_slave.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_frame.h"
#include "content/public/test/layouttest_support.h"

#include "src/common/switches.h"
#include "src/renderer/render_process_observer.h"
#include "src/renderer/render_view_observer.h"
#include "src/renderer/render_frame_observer.h"
#include "src/renderer/extensions/script_context.h"
#include "src/renderer/extensions/module_system.h"
#include "src/renderer/extensions/document_bindings.h"
#include "src/renderer/extensions/web_view_bindings.h"

using namespace content;
using namespace extensions;

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
using blink::WebDataSource;
using blink::WebDocument;
using blink::WebString;
using blink::WebVector;
using blink::WebView;

namespace thrust_shell {

namespace {

ThrustShellRendererClient* g_renderer_client;

}

ThrustShellRendererClient* 
ThrustShellRendererClient::Get() 
{
  return g_renderer_client;
}

ThrustShellRendererClient::ThrustShellRendererClient() 
{
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

ThrustShellRendererClient::~ThrustShellRendererClient() 
{
  g_renderer_client = NULL;
}

void 
ThrustShellRendererClient::RenderThreadStarted() 
{
  observer_.reset(new ThrustShellRenderProcessObserver());
  visited_link_slave_.reset(new visitedlink::VisitedLinkSlave());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif

  RenderThread* thread = RenderThread::Get();

  thread->RegisterExtension(SafeBuiltins::CreateV8Extension());

#include "./extensions/resources/web_view.js.bin"
  std::string web_view_src(
      (char*)src_renderer_extensions_resources_web_view_js,
      src_renderer_extensions_resources_web_view_js_len);
  source_map_.RegisterSource("webview", web_view_src);

  thread->AddObserver(observer_.get());
  thread->AddObserver(visited_link_slave_.get());
}

void 
ThrustShellRendererClient::RenderViewCreated(
    RenderView* render_view) 
{
  new ThrustShellRenderViewObserver(render_view);
}

void 
ThrustShellRendererClient::RenderFrameCreated(
    RenderFrame* render_frame) 
{
  new ThrustShellRenderFrameObserver(render_frame);
}

bool 
ThrustShellRendererClient::OverrideCreatePlugin(
    content::RenderFrame* render_frame,
    blink::WebLocalFrame* frame,
    const WebPluginParams& params,
    WebPlugin** plugin) 
{
  std::string mime_type = params.mimeType.utf8();
  return false;
}

void 
ThrustShellRendererClient::DidCreateScriptContext(
    blink::WebFrame* frame, 
    v8::Handle<v8::Context> v8_context, 
    int extension_group,
    int world_id) 
{
  /* We limit the injection of WebViewBindings to the top level RenderFrames */
  content::RenderFrame* render_frame = 
    content::RenderFrame::FromWebFrame(frame);
  if(render_frame != render_frame->GetRenderView()->GetMainRenderFrame()) {
    return;
  }

  LOG(INFO) << "ThrustShellRendererClient::DidCreateScriptContext `" 
            << frame->uniqueName().utf8() << "` "
            << extension_group << " " 
            << world_id;
  ScriptContext* context = new ScriptContext(v8_context, frame);
  {
    scoped_ptr<ModuleSystem> module_system(new ModuleSystem(context,
                                                            &source_map_));
    context->set_module_system(module_system.Pass());
  }
  ModuleSystem* module_system = context->module_system();

  // Enable natives in startup.
  ModuleSystem::NativesEnabledScope natives_enabled_scope(module_system);

  /* NOTE: please use the naming convention "foo_natives" for these. */
  module_system->RegisterNativeHandler("document_natives",
      scoped_ptr<NativeHandler>(
          new DocumentBindings(context)));
  module_system->RegisterNativeHandler("webview_natives",
      scoped_ptr<NativeHandler>(
          new WebViewBindings(context)));

  module_system->Require("webview");
}

bool 
ThrustShellRendererClient::ShouldFork(
    blink::WebFrame* frame,
    const GURL& url,
    const std::string& http_method,
    bool is_initial_navigation,
    bool is_server_redirect,
    bool* send_referrer) 
{
  return false;
}


unsigned long long 
ThrustShellRendererClient::VisitedLinkHash(
    const char* canonical_url, 
    size_t length) 
{ 
  return visited_link_slave_->ComputeURLFingerprint(canonical_url, length);
}

bool 
ThrustShellRendererClient::IsLinkVisited(
    unsigned long long link_hash)
{
  //LOG(INFO) << link_hash << " " << visited_link_slave_->IsVisited(link_hash);
  return visited_link_slave_->IsVisited(link_hash);
}

} // namespace thrust_shell
