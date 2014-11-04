// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/extensions/dispatcher.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebURLRequest.h"
#include "third_party/WebKit/public/web/WebCustomElement.h"
#include "third_party/WebKit/public/web/WebDataSource.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebRuntimeFeatures.h"
#include "third_party/WebKit/public/web/WebScopedUserGesture.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "third_party/WebKit/public/web/WebUserGestureIndicator.h"
#include "third_party/WebKit/public/web/WebView.h"

#include "src/renderer/extensions/script_context.h"
#include "src/renderer/extensions/module_system.h"
#include "src/renderer/extensions/document_bindings.h"
#include "src/renderer/extensions/web_view_bindings.h"

using blink::WebDataSource;
using blink::WebDocument;
using blink::WebFrame;
using blink::WebScopedUserGesture;
using blink::WebSecurityPolicy;
using blink::WebString;
using blink::WebVector;
using blink::WebView;
using content::RenderThread;
using content::RenderView;

namespace extensions {

Dispatcher::Dispatcher()
  : is_webkit_initialized_(false)
{
  RenderThread::Get()->RegisterExtension(SafeBuiltins::CreateV8Extension());
  PopulateSourceMap();
}

Dispatcher::~Dispatcher() {
}

void 
Dispatcher::PopulateSourceMap() 
{

  /*
#include "./resources/test_view.js.bin"
  std::string test_view_src((char*)test_view_js, test_view_js_len);
  LOG(INFO) << test_view_src;
  source_map_.RegisterSource("testview", test_view_src);
   */

  //source_map_.RegisterSource("webView", IDR_WEB_VIEW_JS);
#include "./resources/web_view.js.bin"
  std::string web_view_src(
      (char*)src_renderer_extensions_resources_web_view_js,
      src_renderer_extensions_resources_web_view_js_len);
  source_map_.RegisterSource("webview", web_view_src);

  /*
  // Note: webView not webview so that this doesn't interfere with the
  // chrome.webview API bindings.
  //source_map_.RegisterSource("webview", IDR_WEBVIEW_CUSTOM_BINDINGS_JS);
  source_map_.RegisterSource("webViewExperimental",
                             IDR_WEB_VIEW_EXPERIMENTAL_JS);
  source_map_.RegisterSource("webViewRequest",
                             IDR_WEB_VIEW_REQUEST_CUSTOM_BINDINGS_JS);
  source_map_.RegisterSource("denyWebView", IDR_WEB_VIEW_DENY_JS);
  source_map_.RegisterSource("adView", IDR_AD_VIEW_JS);
  source_map_.RegisterSource("denyAdView", IDR_AD_VIEW_DENY_JS);
  source_map_.RegisterSource("platformApp", IDR_PLATFORM_APP_JS);
  source_map_.RegisterSource("injectAppTitlebar", IDR_INJECT_APP_TITLEBAR_JS);
  */
}


void 
Dispatcher::DidCreateScriptContext(
    WebFrame* frame, 
    v8::Handle<v8::Context> v8_context, 
    int extension_group,
    int world_id) 
{
  ScriptContext* context = new ScriptContext(v8_context, frame);
  //v8_context_set_.Add(context);

  {
    scoped_ptr<ModuleSystem> module_system(new ModuleSystem(context,
                                                            &source_map_));
    context->set_module_system(module_system.Pass());
  }
  ModuleSystem* module_system = context->module_system();

  // Enable natives in startup.
  ModuleSystem::NativesEnabledScope natives_enabled_scope(module_system);
  RegisterNativeHandlers(module_system, context);

  /*
  module_system->RegisterNativeHandler("chrome",
      scoped_ptr<NativeHandler>(new ChromeNativeHandler(context)));
  module_system->RegisterNativeHandler("print",
      scoped_ptr<NativeHandler>(new PrintNativeHandler(context)));
  module_system->RegisterNativeHandler("lazy_background_page",
      scoped_ptr<NativeHandler>(
          new LazyBackgroundPageNativeHandler(this, context)));
  module_system->RegisterNativeHandler("logging",
      scoped_ptr<NativeHandler>(new LoggingNativeHandler(context)));
  module_system->RegisterNativeHandler("schema_registry",
      v8_schema_registry_->AsNativeHandler());
  module_system->RegisterNativeHandler("v8_context",
      scoped_ptr<NativeHandler>(new V8ContextNativeHandler(context, this)));
  module_system->RegisterNativeHandler("test_features",
      scoped_ptr<NativeHandler>(new TestFeaturesNativeHandler(context)));
  module_system->RegisterNativeHandler("user_gestures",
      scoped_ptr<NativeHandler>(new UserGesturesNativeHandler(context)));
  module_system->RegisterNativeHandler("utils",
      scoped_ptr<NativeHandler>(new UtilsNativeHandler(context)));

  module_system->RegisterNativeHandler("process",
      scoped_ptr<NativeHandler>(new ProcessInfoNativeHandler(
          this, context, context->GetExtensionID(),
          context->GetContextTypeDescription(),
          ChromeRenderProcessObserver::is_incognito_process(),
          manifest_version, send_request_disabled)));
  */

  module_system->Require("webview");
  LOG(INFO) << "Module requires called!";

  //VLOG(1) << "Num tracked contexts: " << v8_context_set_.size();
}

// NOTE: please use the naming convention "foo_natives" for these.
void 
Dispatcher::RegisterNativeHandlers(
    ModuleSystem* module_system,
    ScriptContext* context) 
{
  module_system->RegisterNativeHandler("document_natives",
      scoped_ptr<NativeHandler>(
          new DocumentBindings(context)));
  module_system->RegisterNativeHandler("webview_natives",
      scoped_ptr<NativeHandler>(
          new WebViewBindings(context)));
  /*
  module_system->RegisterNativeHandler("event_natives",
      scoped_ptr<NativeHandler>(EventBindings::Create(this, context)));
  module_system->RegisterNativeHandler("messaging_natives",
      scoped_ptr<NativeHandler>(MessagingBindings::Get(this, context)));
  */
  /*
  module_system->RegisterNativeHandler("apiDefinitions",
      scoped_ptr<NativeHandler>(new ApiDefinitionsNatives(this, context)));
  module_system->RegisterNativeHandler("sendRequest",
      scoped_ptr<NativeHandler>(
          new SendRequestNatives(this, request_sender_.get(), context)));
  module_system->RegisterNativeHandler("setIcon",
      scoped_ptr<NativeHandler>(
          new SetIconNatives(this, request_sender_.get(), context)));
  module_system->RegisterNativeHandler("activityLogger",
      scoped_ptr<NativeHandler>(new APIActivityLogger(this, context)));
  module_system->RegisterNativeHandler("renderViewObserverNatives",
      scoped_ptr<NativeHandler>(new RenderViewObserverNatives(this, context)));

  // Natives used by multiple APIs.
  module_system->RegisterNativeHandler("file_system_natives",
      scoped_ptr<NativeHandler>(new FileSystemNatives(context)));

  // Custom bindings.
  module_system->RegisterNativeHandler("app",
      scoped_ptr<NativeHandler>(new AppBindings(this, context)));
  module_system->RegisterNativeHandler("app_runtime",
      scoped_ptr<NativeHandler>(
          new AppRuntimeCustomBindings(this, context)));
  module_system->RegisterNativeHandler("app_window_natives",
      scoped_ptr<NativeHandler>(
          new AppWindowCustomBindings(this, context)));
  module_system->RegisterNativeHandler("blob_natives",
      scoped_ptr<NativeHandler>(new BlobNativeHandler(context)));
  module_system->RegisterNativeHandler("context_menus",
      scoped_ptr<NativeHandler>(
          new ContextMenusCustomBindings(this, context)));
  module_system->RegisterNativeHandler(
      "css_natives", scoped_ptr<NativeHandler>(new CssNativeHandler(context)));
  module_system->RegisterNativeHandler("sync_file_system",
      scoped_ptr<NativeHandler>(
          new SyncFileSystemCustomBindings(this, context)));
  module_system->RegisterNativeHandler("file_browser_handler",
      scoped_ptr<NativeHandler>(new FileBrowserHandlerCustomBindings(
          this, context)));
  module_system->RegisterNativeHandler("file_browser_private",
      scoped_ptr<NativeHandler>(new FileBrowserPrivateCustomBindings(
          this, context)));
  module_system->RegisterNativeHandler("i18n",
      scoped_ptr<NativeHandler>(
          new I18NCustomBindings(this, context)));
  module_system->RegisterNativeHandler(
      "id_generator",
      scoped_ptr<NativeHandler>(new IdGeneratorCustomBindings(this, context)));
  module_system->RegisterNativeHandler("mediaGalleries",
      scoped_ptr<NativeHandler>(
          new MediaGalleriesCustomBindings(this, context)));
  module_system->RegisterNativeHandler("page_actions",
      scoped_ptr<NativeHandler>(
          new PageActionsCustomBindings(this, context)));
  module_system->RegisterNativeHandler("page_capture",
      scoped_ptr<NativeHandler>(
          new PageCaptureCustomBindings(this, context)));
  module_system->RegisterNativeHandler(
      "pepper_request_natives",
      scoped_ptr<NativeHandler>(new PepperRequestNatives(context)));
  module_system->RegisterNativeHandler("runtime",
      scoped_ptr<NativeHandler>(new RuntimeCustomBindings(this, context)));
  module_system->RegisterNativeHandler("tabs",
      scoped_ptr<NativeHandler>(new TabsCustomBindings(this, context)));
  module_system->RegisterNativeHandler("webstore",
      scoped_ptr<NativeHandler>(new WebstoreBindings(this, context)));
#if defined(ENABLE_WEBRTC)
  module_system->RegisterNativeHandler("cast_streaming_natives",
      scoped_ptr<NativeHandler>(new CastStreamingNativeHandler(context)));
#endif
  */
}

void 
Dispatcher::WillReleaseScriptContext(
    WebFrame* frame, 
    v8::Handle<v8::Context> v8_context, 
    int world_id) 
{
  /*
  ScriptContext* context = v8_context_set_.GetByV8Context(v8_context);
  if (!context)
    return;

  // If the V8 context has an OOM exception, javascript execution has been
  // stopped, so dispatching an onUnload event is pointless.
  if (!v8_context->HasOutOfMemoryException())
    context->DispatchOnUnloadEvent();
  // TODO(kalman): add an invalidation observer interface to ChromeV8Context.
  request_sender_->InvalidateSource(context);

  v8_context_set_.Remove(context);
  VLOG(1) << "Num tracked contexts: " << v8_context_set_.size();
  */

  /* TODO(spolu): Collect context. */
}

void 
Dispatcher::DidCreateDocumentElement(
    blink::WebFrame* frame) 
{
  /*
  content_watcher_->DidCreateDocumentElement(frame);
  */
}

void 
Dispatcher::WebKitInitialized() 
{
  EnableCustomElementWhiteList();
  is_webkit_initialized_ = true;
}

void 
Dispatcher::IdleNotification() 
{
}


void 
Dispatcher::EnableCustomElementWhiteList()
{
  blink::WebCustomElement::addEmbedderCustomElementName("browserplugin"); 
  blink::WebCustomElement::addEmbedderCustomElementName("webview");
}

void 
Dispatcher::OnRenderProcessShutdown() 
{
  /*
  v8_schema_registry_.reset();
  */
}

} // namespace extensions
