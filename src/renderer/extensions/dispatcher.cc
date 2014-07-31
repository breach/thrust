// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/extensions/dispatcher.h"

#include "base/callback.h"
#include "base/command_line.h"

#include "src/renderer/extensions/local_source_map.h"

using base::UserMetricsAction;
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
  : is_webkit_initialized_(false),
    source_map_(new LocalSourceMap())
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
  source_map_.RegisterSource("webview", IDR_WEBVIEW_CUSTOM_BINDINGS_JS);
  // Note: webView not webview so that this doesn't interfere with the
  // chrome.webview API bindings.
  source_map_.RegisterSource("webView", IDR_WEB_VIEW_JS);
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
  Context* context = new Context(v8_context, frame);
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

  AddOrRemoveBindingsForContext(context);

  module_system->Require("webView");
  //module_system->Require("webViewExperimental");

  VLOG(1) << "Num tracked contexts: " << v8_context_set_.size();
}

void 
Dispatcher::WillReleaseScriptContext(
    WebFrame* frame, 
    v8::Handle<v8::Context> v8_context, 
    int world_id) 
{
  /*
  Context* context = v8_context_set_.GetByV8Context(v8_context);
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

bool 
Dispatcher::OnControlMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(Dispatcher, message)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
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
  blink::WebCustomElement::addEmbedderCustomElementName("frame");
}

void 
Dispatcher::OnRenderProcessShutdown() 
{
  /*
  v8_schema_registry_.reset();
  */
}

} // namespace extensions
