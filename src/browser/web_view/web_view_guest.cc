// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/browser/web_view/web_view_guest.h"

#include "base/lazy_instance.h"
#include "base/process/kill.h"
#include "base/process/process_handle.h" 
#include "base/strings/utf_string_conversions.h"
#include "net/base/escape.h"
#include "net/base/net_errors.h"
#include "content/public/common/result_codes.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/host_zoom_map.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/resource_request_details.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/common/page_zoom.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/WebKit/public/web/WebFindOptions.h"

#include "src/browser/web_view/web_view_constants.h"
#include "src/browser/browser_client.h"
#include "src/browser/session/thrust_session.h"
#include "src/browser/thrust_window.h"

using content::WebContents;

namespace {

// WebContents* => WebViewGuest*
typedef std::map<WebContents*, thrust_shell::WebViewGuest*> 
  WebContentsWebViewGuestMap;
static base::LazyInstance<WebContentsWebViewGuestMap> webcontents_webview_map =
    LAZY_INSTANCE_INITIALIZER;

std::string WindowOpenDispositionToString(
  WindowOpenDisposition window_open_disposition) {
  switch (window_open_disposition) {
    case IGNORE_ACTION:
      return "ignore";
    case SAVE_TO_DISK:
      return "save_to_disk";
    case CURRENT_TAB:
      return "current_tab";
    case NEW_BACKGROUND_TAB:
      return "new_background_tab";
    case NEW_FOREGROUND_TAB:
      return "new_foreground_tab";
    case NEW_WINDOW:
      return "new_window";
    case NEW_POPUP:
      return "new_popup";
    default:
      NOTREACHED() << "Unknown Window Open Disposition";
      return "ignore";
  }
}

static std::string TerminationStatusToString(base::TerminationStatus status) {
  switch (status) {
    case base::TERMINATION_STATUS_NORMAL_TERMINATION:
      return "normal";
    case base::TERMINATION_STATUS_ABNORMAL_TERMINATION:
    case base::TERMINATION_STATUS_STILL_RUNNING:
      return "abnormal";
    case base::TERMINATION_STATUS_PROCESS_WAS_KILLED:
      return "killed";
    case base::TERMINATION_STATUS_PROCESS_CRASHED:
      return "crashed";
    case base::TERMINATION_STATUS_MAX_ENUM:
      break;
  }
  NOTREACHED() << "Unknown Termination Status.";
  return "unknown";
}
}  // namespace

namespace thrust_shell {

// This observer ensures that the WebViewGuest destroys itself when its
// embedder goes away.
class WebViewGuest::EmbedderWebContentsObserver : public WebContentsObserver {
 public:
  explicit EmbedderWebContentsObserver(WebViewGuest* guest)
      : WebContentsObserver(guest->embedder_web_contents()),
        destroyed_(false),
        guest_(guest) {
  }

  virtual ~EmbedderWebContentsObserver() {
  }

  // WebContentsObserver implementation.
  virtual void WebContentsDestroyed() OVERRIDE {
    Destroy();
  }

  virtual void RenderProcessGone(base::TerminationStatus status) OVERRIDE {
    Destroy();
  }

 private:
  void Destroy() {
    if(destroyed_) {
      return;
    }
    destroyed_ = true;
    guest_->embedder_web_contents_ = NULL;
    //guest_->EmbedderDestroyed();
    guest_->Destroy();
  }

  bool          destroyed_;
  WebViewGuest* guest_;

  DISALLOW_COPY_AND_ASSIGN(EmbedderWebContentsObserver);
};


WebViewGuest::WebViewGuest(
    int guest_instance_id)
: embedder_web_contents_(NULL),
  embedder_render_process_id_(0),
  browser_context_(NULL),
  guest_instance_id_(guest_instance_id),
  view_instance_id_(webview::kInstanceIDNone),
  auto_size_enabled_(false),
  weak_ptr_factory_(this) 
{
  LOG(INFO) << "WebViewGuest Constructor: " << this;
}

void
WebViewGuest::Init(
    WebContents* guest_web_contents)
{
  WebContentsObserver::Observe(guest_web_contents);
  guest_web_contents_.reset(
      brightray::InspectableWebContents::Create(guest_web_contents));
  guest_web_contents->SetDelegate(this);
  browser_context_ = guest_web_contents->GetBrowserContext();

  webcontents_webview_map.Get().insert(
      std::make_pair(guest_web_contents, this));

  notification_registrar_.Add(
      this, content::NOTIFICATION_LOAD_COMPLETED_MAIN_FRAME,
      content::Source<WebContents>(guest_web_contents));

  notification_registrar_.Add(
      this, content::NOTIFICATION_RESOURCE_RECEIVED_REDIRECT,
      content::Source<WebContents>(guest_web_contents));

  ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(browser_context_)->
    AddGuest(guest_instance_id_, guest_web_contents);

  LOG(INFO) << "WebViewGuest Init: " << this;
}


// static
WebViewGuest* 
WebViewGuest::Create(
    int guest_instance_id)
{
  return new WebViewGuest(guest_instance_id);
}

// static
WebViewGuest* 
WebViewGuest::FromWebContents(
    WebContents* web_contents) 
{
  WebContentsWebViewGuestMap* webview_map = webcontents_webview_map.Pointer();
  WebContentsWebViewGuestMap::iterator it = webview_map->find(web_contents);
  return it == webview_map->end() ? NULL : it->second;
}

// static.
int 
WebViewGuest::GetViewInstanceId(
    WebContents* contents) 
{
  WebViewGuest* guest = FromWebContents(contents);
  if(!guest)
    return webview::kInstanceIDNone;

  return guest->view_instance_id();
}

// static
void 
WebViewGuest::ParsePartitionParam(
    const base::DictionaryValue* extra_params,
    std::string* storage_partition_id,
    bool* persist_storage) 
{
  std::string partition_str;
  if (!extra_params->GetString(webview::kStoragePartitionId, &partition_str)) {
    return;
  }

  // Since the "persist:" prefix is in ASCII, StartsWith will work fine on
  // UTF-8 encoded |partition_id|. If the prefix is a match, we can safely
  // remove the prefix without splicing in the middle of a multi-byte codepoint.
  // We can use the rest of the string as UTF-8 encoded one.
  if (StartsWithASCII(partition_str, "persist:", true)) {
    size_t index = partition_str.find(":");
    CHECK(index != std::string::npos);
    // It is safe to do index + 1, since we tested for the full prefix above.
    *storage_partition_id = partition_str.substr(index + 1);

    if (storage_partition_id->empty()) {
      // TODO(lazyboy): Better way to deal with this error.
      return;
    }
    *persist_storage = true;
  } else {
    *storage_partition_id = partition_str;
    *persist_storage = false;
  }
}

/******************************************************************************/
/* BROWSER_PLUGIN_GUEST_DELEGATE IMPLEMENTATION */
/******************************************************************************/
void 
WebViewGuest::Destroy() 
{
  LOG(INFO) << "WebViewGuest Destroy: " << this;

  if(!destruction_callback_.is_null())
    destruction_callback_.Run();

  webcontents_webview_map.Get().erase(guest_web_contents());

  ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(browser_context_)->
    RemoveGuest(guest_instance_id_);

  guest_web_contents_.reset();
}

void 
WebViewGuest::DidAttach() 
{
  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-attach",
      *(extra_params_.get()));
}

void 
WebViewGuest::ElementSizeChanged(
    const gfx::Size& old_size,
    const gfx::Size& new_size) 
{
  element_size_ = new_size;
}

int 
WebViewGuest::GetGuestInstanceID() const 
{
  return guest_instance_id_;
}

void 
WebViewGuest::GuestSizeChanged(
    const gfx::Size& old_size,
    const gfx::Size& new_size) 
{
  if (!auto_size_enabled_)
    return;
  guest_size_ = new_size;
  //GuestSizeChangedDueToAutoSize(old_size, new_size);
}


void 
WebViewGuest::RegisterDestructionCallback(
    const DestructionCallback& callback) 
{
  destruction_callback_ = callback;
}

void 
WebViewGuest::WillAttach(
    content::WebContents* embedder_web_contents,
    const base::DictionaryValue& extra_params) 
{
  embedder_web_contents_ = embedder_web_contents;
  embedder_web_contents_observer_.reset(
      new EmbedderWebContentsObserver(this));
  embedder_render_process_id_ =
      embedder_web_contents->GetRenderProcessHost()->GetID();
  extra_params.GetInteger(webview::kParameterInstanceId, &view_instance_id_);
  extra_params_.reset(extra_params.DeepCopy());

  LOG(INFO) << "WebViewGuest WillAttach: " << embedder_web_contents << " " << view_instance_id_;
}

content::WebContents* 
WebViewGuest::CreateNewGuestWindow(
    const content::WebContents::CreateParams& create_params) 
{
  NOTREACHED() << "Should not create new window from guest";
  return NULL;
}

WebViewGuest::~WebViewGuest() 
{
  LOG(INFO) << "WebViewGuest Destructor: " << this;
}

void 
WebViewGuest::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) 
{
  switch (type) {
    case content::NOTIFICATION_LOAD_COMPLETED_MAIN_FRAME: {
      DCHECK_EQ(content::Source<WebContents>(source).ptr(),
                guest_web_contents());
      if(content::Source<WebContents>(source).ptr() == guest_web_contents()) {
        //LoadHandlerCalled();
      }
      break;
    }
    case content::NOTIFICATION_RESOURCE_RECEIVED_REDIRECT: {
      DCHECK_EQ(content::Source<WebContents>(source).ptr(),
                guest_web_contents());
      /*
      content::ResourceRedirectDetails* resource_redirect_details =
          content::Details<content::ResourceRedirectDetails>(details).ptr();
      bool is_top_level =
          resource_redirect_details->resource_type == ResourceType::MAIN_FRAME;
      LoadRedirect(resource_redirect_details->url,
                   resource_redirect_details->new_url,
                   is_top_level);
      */
      break;
    }
    default:
      NOTREACHED() << "Unexpected notification sent.";
      break;
  }
}

/******************************************************************************/
/* WEBVIEW API */
/******************************************************************************/
void
WebViewGuest::LoadUrl(
    const GURL& url)
{
  content::NavigationController::LoadURLParams params(url);
  params.transition_type = content::PageTransitionFromInt(
      content::PAGE_TRANSITION_TYPED | 
      content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  params.override_user_agent = content::NavigationController::UA_OVERRIDE_TRUE; 
  guest_web_contents()->GetController().LoadURLWithParams(params);
}


void 
WebViewGuest::SetZoom(
    double zoom_factor) 
{
  double zoom_level = content::ZoomFactorToZoomLevel(zoom_factor);
  content::HostZoomMap::SetZoomLevel(guest_web_contents(), zoom_level);

  base::DictionaryValue event;
  event.SetDouble("old_zoom_factor", current_zoom_factor_);
  event.SetDouble("new_zoom_factor", zoom_factor);

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "zoom-changed",
      event);

  current_zoom_factor_ = zoom_factor;
}

double 
WebViewGuest::GetZoom() 
{
  return current_zoom_factor_;
}

void 
WebViewGuest::Go(
    int relative_index) 
{
  guest_web_contents()->GetController().GoToOffset(relative_index);
}

void 
WebViewGuest::Reload(bool ignore_cache) 
{
  // TODO(fsamuel): Don't check for repost because we don't want to show
  // Chromium's repost warning. We might want to implement a separate API
  // for registering a callback if a repost is about to happen.
  if(ignore_cache) {
    guest_web_contents()->GetController().ReloadIgnoringCache(false);
  }
  else {
    guest_web_contents()->GetController().Reload(false);
  }
}

void 
WebViewGuest::Stop() 
{
  guest_web_contents()->Stop();
}

void
WebViewGuest::Terminate() 
{
  base::ProcessHandle process_handle =
      guest_web_contents()->GetRenderProcessHost()->GetHandle();
  if (process_handle)
    base::KillProcess(process_handle, content::RESULT_CODE_KILLED, false);
}


void 
WebViewGuest::Find(
    int request_id, 
    const std::string& search_text,
    const blink::WebFindOptions& options)
{
  base::string16 text = base::UTF8ToUTF16(search_text);
  guest_web_contents()->Find(request_id, text, options);
}

void 
WebViewGuest::StopFinding(
    content::StopFindAction action)
{
  guest_web_contents()->StopFinding(action);
}

void 
WebViewGuest::InsertCSS(
    const std::string& css)
{
  guest_web_contents()->InsertCSS(css);
}

void 
WebViewGuest::ExecuteScript(
    const std::string& script)
{
  base::string16 code = base::UTF8ToUTF16(script);
  guest_web_contents()->GetMainFrame()->ExecuteJavaScript(code);
}

void 
WebViewGuest::OpenDevTools()
{
  LOG(INFO) << "SHOW DEV TOOL *******************";
  //guest_web_contents_.get()->SetCanDock(false);
  guest_web_contents_.get()->ShowDevTools();
}

void 
WebViewGuest::CloseDevTools()
{
  guest_web_contents_.get()->CloseDevTools();
}

bool
WebViewGuest::IsDevToolsOpened()
{
  return guest_web_contents_.get()->IsDevToolsViewShowing();
}

/******************************************************************************/
/* PUBLIC API */
/******************************************************************************/
void 
WebViewGuest::SetAutoSize(
    bool enabled,
    const gfx::Size& min_size,
    const gfx::Size& max_size) 
{
  min_auto_size_ = min_size;
  min_auto_size_.SetToMin(max_size);
  max_auto_size_ = max_size;
  max_auto_size_.SetToMax(min_size);

  enabled &= !min_auto_size_.IsEmpty() && !max_auto_size_.IsEmpty();
  if (!enabled && !auto_size_enabled_)
    return;

  auto_size_enabled_ = enabled;

  if(!attached())
    return;

  content::RenderViewHost* rvh = guest_web_contents()->GetRenderViewHost();
  if (auto_size_enabled_) {
    rvh->EnableAutoResize(min_auto_size_, max_auto_size_);
  } else {
    rvh->DisableAutoResize(element_size_);
    guest_size_ = element_size_;
    //GuestSizeChangedDueToAutoSize(guest_size_, element_size_);
  }
}

/******************************************************************************/
/* PRIVATE & PROTECTED API */
/******************************************************************************/
ThrustWindow*
WebViewGuest::GetThrustWindow()
{
  std::vector<ThrustWindow*> instances = ThrustWindow::instances();
  for(size_t i = 0; i < instances.size(); ++i) {
    if(instances[i]->GetWebContents() == embedder_web_contents_) {
      return instances[i];
    }
  }
  return NULL;
}

/******************************************************************************/
/* WEBCONTENTSOBSERVER IMPLEMENTATION */
/******************************************************************************/
void 
WebViewGuest::RenderViewReady() 
{
  //GuestReady();
  content::RenderViewHost* rvh = guest_web_contents()->GetRenderViewHost();
  if (auto_size_enabled_) {
    rvh->EnableAutoResize(min_auto_size_, max_auto_size_);
  } 
  else {
    rvh->DisableAutoResize(element_size_);
  }
}

void 
WebViewGuest::WebContentsDestroyed() 
{
  base::DictionaryValue event;

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "destroyed",
      event);

  delete this;
}



void 
WebViewGuest::DidFinishLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url) 
{
  bool is_main_frame = !render_frame_host->GetParent();

  base::DictionaryValue event;
  event.SetBoolean("is_top_level", !render_frame_host->GetParent());
  event.SetString("url", validated_url.spec());

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-frame-finish-load",
      event);
  if(is_main_frame) {
    GetThrustWindow()->WebViewEmit(
        guest_instance_id_,
        "did-finish-load",
        event);
  }
}

void 
WebViewGuest::DidFailLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code,
    const base::string16& error_description) 
{
  base::DictionaryValue event;
  event.SetString("url", validated_url.spec());
  event.SetBoolean("is_top_level", !render_frame_host->GetParent());
  event.SetInteger("error_code", error_code);
  event.SetString("error_description", error_description);

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-fail-load",
      event);
}

void 
WebViewGuest::DidStartLoading(
    content::RenderViewHost* render_view_host) 
{
  base::DictionaryValue event;
  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-start-loading",
      event);
}

void 
WebViewGuest::DidStopLoading(
    content::RenderViewHost* render_view_host) 
{
  base::DictionaryValue event;
  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-stop-loading",
      event);
}

void 
WebViewGuest::DidGetRedirectForResourceRequest(
    content::RenderViewHost* render_view_host,
    const content::ResourceRedirectDetails& details) 
{
  base::DictionaryValue event;
  event.SetString("current_url", details.url.spec());
  event.SetString("new_url", details.new_url.spec());
  event.SetBoolean("is_top_level", 
                   details.resource_type == content::RESOURCE_TYPE_MAIN_FRAME);

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-get-redirect-request",
      event);
}





void 
WebViewGuest::DidCommitProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& url,
    content::PageTransition transition_type) 
{
  //find_helper_.CancelAllFindSessions();

  base::DictionaryValue event;
  event.SetString("url", url.spec());
  event.SetBoolean("is_top_level", !render_frame_host->GetParent());
  event.SetInteger("entry_index",
                   guest_web_contents()->GetController().GetCurrentEntryIndex());
  event.SetInteger("entry_count",
                   guest_web_contents()->GetController().GetEntryCount());
  event.SetInteger("process_id",
                   guest_web_contents()->GetRenderProcessHost()->GetID());

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-commit-provisional-load",
      event);

  double zoom_factor = blink::WebView::zoomLevelToZoomFactor(
      content::HostZoomMap::GetZoomLevel(guest_web_contents()));
  if(current_zoom_factor_ != zoom_factor) {
    base::DictionaryValue event;
    event.SetDouble("old_zoom_factor", current_zoom_factor_);
    event.SetDouble("new_zoom_factor", zoom_factor);

    GetThrustWindow()->WebViewEmit(
        guest_instance_id_,
        "zoom-changed",
        event);
    current_zoom_factor_ = zoom_factor;
  }
}

void 
WebViewGuest::DidFailProvisionalLoad(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    int error_code,
    const base::string16& error_description) 
{
  base::DictionaryValue event;
  event.SetBoolean("is_top_level", !render_frame_host->GetParent());
  event.SetString("url", validated_url.spec());
  event.SetString("error_type", net::ErrorToShortString(error_code));

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-fail-provisional-load",
      event);
}

void 
WebViewGuest::DidStartProvisionalLoadForFrame(
    content::RenderFrameHost* render_frame_host,
    const GURL& validated_url,
    bool is_error_page,
    bool is_iframe_srcdoc) 
{
  base::DictionaryValue event;
  event.SetString("url", validated_url.spec());
  event.SetBoolean("is_top_level", !render_frame_host->GetParent());

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "did-start-provisional-load",
      event);
}

void 
WebViewGuest::UserAgentOverrideSet(
    const std::string& user_agent) 
{
  if(!attached()) {
    return;
  }
  content::NavigationController& controller =
      guest_web_contents()->GetController();
  content::NavigationEntry* entry = controller.GetVisibleEntry();
  if(!entry) {
    return;
  }
  entry->SetIsOverridingUserAgent(!user_agent.empty());
  guest_web_contents()->GetController().Reload(false);
}

void 
WebViewGuest::RenderProcessGone(
    base::TerminationStatus status) 
{
  // Cancel all find sessions in progress.
  // find_helper_.CancelAllFindSessions();

  base::DictionaryValue event;
  event.SetInteger("process_id",
                   guest_web_contents()->GetRenderProcessHost()->GetID());
  event.SetString("reason", TerminationStatusToString(status));

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "crashed",
      event);
}



/******************************************************************************/
/* WEBCONTENTSDELEGATE IMPLEMENTATION */
/******************************************************************************/
bool 
WebViewGuest::AddMessageToConsole(
    content::WebContents* source,
    int32 level,
    const base::string16& message,
    int32 line_no,
    const base::string16& source_id) 
{
  base::DictionaryValue event;
  event.SetInteger("level", level);
  event.SetString("message", message);
  event.SetInteger("line", line_no);
  event.SetString("source_id", source_id);

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "console",
      event);
  return true;
}

bool 
WebViewGuest::ShouldCreateWebContents(
    content::WebContents* web_contents,
    int route_id,
    WindowContainerType window_container_type,
    const base::string16& frame_name,
    const GURL& target_url,
    const std::string& partition_id,
    content::SessionStorageNamespace* session_storage_namespace) 
{
  base::DictionaryValue event;
  event.SetString("target_url", target_url.spec());
  event.SetString("disposition", 
      WindowOpenDispositionToString(NEW_FOREGROUND_TAB));
  event.SetString("frame_name", frame_name);
  event.SetInteger("window_container_type", window_container_type);

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "new-window",
      event);
  return false;
}

void 
WebViewGuest::CloseContents(
    content::WebContents* source) 
{
  base::DictionaryValue event;

  GetThrustWindow()->WebViewEmit(
      guest_instance_id_,
      "close",
      event);
}

content::WebContents* 
WebViewGuest::OpenURLFromTab(
    content::WebContents* source,
    const content::OpenURLParams& params) 
{
  if(params.disposition == CURRENT_TAB) {
    content::NavigationController::LoadURLParams load_url_params(params.url);
    load_url_params.referrer = params.referrer;
    load_url_params.transition_type = params.transition;
    load_url_params.extra_headers = params.extra_headers;
    load_url_params.should_replace_current_entry =
      params.should_replace_current_entry;
    load_url_params.is_renderer_initiated = params.is_renderer_initiated;
    load_url_params.transferred_global_request_id =
      params.transferred_global_request_id;

    guest_web_contents()->GetController().LoadURLWithParams(load_url_params);
    return guest_web_contents();
  }
  else {
    base::DictionaryValue event;
    event.SetString("target_url", params.url.spec());
    event.SetString("disposition", 
        WindowOpenDispositionToString(params.disposition));

    GetThrustWindow()->WebViewEmit(
        guest_instance_id_,
        "new-window",
        event);
    return NULL;
  }
}

void 
WebViewGuest::HandleKeyboardEvent(
    content::WebContents* source,
    const content::NativeWebKeyboardEvent& event) 
{
  if(!attached())
    return;

  /* TODO(spolu): emit event? */

  // Send the unhandled keyboard events back to the embedder to reprocess them.
  embedder_web_contents_->GetDelegate()->HandleKeyboardEvent(
      guest_web_contents(), event);
}


} // namespace thrust_shell
