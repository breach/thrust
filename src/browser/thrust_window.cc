// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/thrust_window.h"

#include "base/auto_reset.h"
#include "base/command_line.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/file_util.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/codec/jpeg_codec.h"
#include "content/public/common/url_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/favicon_status.h"
#include "third_party/WebKit/public/web/WebFindOptions.h"

#include "src/common/switches.h"
#include "src/browser/browser_main_parts.h"
#include "src/browser/browser_client.h"
#include "src/browser/dialog/javascript_dialog_manager.h"
#include "src/browser/dialog/web_dialog_helper.h"
#include "src/browser/web_view/web_view_guest.h"
#include "src/browser/session/thrust_session.h"
#include "src/common/messages.h"
#include "src/browser/ui/views/menu_bar.h"
#include "src/browser/ui/views/menu_layout.h"
#include "src/api/thrust_window_binding.h"
#include "src/browser/dialog/browser_dialogs.h"

#if defined(USE_X11)
#include "src/browser/ui/views/global_menu_bar_x11.h"
#elif defined(OS_WIN)
#endif

using namespace content;

namespace thrust_shell {

std::vector<ThrustWindow*> ThrustWindow::s_instances;

/******************************************************************************/
/* CONSTRUCTOR / DESTRUCTOR */
/******************************************************************************/
ThrustWindow::ThrustWindow(
    ThrustWindowBinding* binding,
    WebContents* web_contents,
    const gfx::Size& size,
    const std::string& title,
    const std::string& icon_path,
    const bool has_frame)
  : WebContentsObserver(web_contents),
    binding_(binding),
    is_closed_(false),
    title_(title),
    has_frame_(has_frame),
    inspectable_web_contents_(
        brightray::InspectableWebContents::Create(web_contents))
{
  web_contents->SetDelegate(this);
  inspectable_web_contents()->SetDelegate(this);
  //WebContentsObserver::Observe(web_contents);

  // Get notified of title updated message.
  registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
                 Source<WebContents>(web_contents));

  /* Set the icon for the window before it gets displayed. We exceptionally */
  /* allow IO on this thread (UI) to load the PNG image.                    */
  base::ThreadRestrictions::SetIOAllowed(true);
  gfx::Image icon;
  base::FilePath p = base::FilePath::FromUTF8Unsafe(icon_path);
  // Read the file from disk.
  std::string file_contents;
  if(!p.empty() && base::ReadFileToString(p, &file_contents)) {
    // Decode the bitmap using WebKit's image decoder.
    const unsigned char* data =
      reinterpret_cast<const unsigned char*>(file_contents.data());
    size_t size = file_contents.size();
    scoped_ptr<SkBitmap> decoded(new SkBitmap());

    if(!gfx::PNGCodec::Decode(data, size, decoded.get())) {
      decoded.reset(gfx::JPEGCodec::Decode(data, size));                             
    }
    if(decoded) {                                                                   
      icon_.AddRepresentation(gfx::ImageSkiaRep(
            *decoded.release(), 1.0f));     
    }                                  
  }
  base::ThreadRestrictions::SetIOAllowed(false);

  /*
  renderer_preferences_util::UpdateFromSystemSettings(
      web_contents_->GetMutableRendererPrefs());
  web_contents_->GetRenderViewHost()->SyncRendererPrefs();
   */
  PlatformCreateWindow(size);

  LOG(INFO) << "ThrustWindow Constructor [" << web_contents << "]";
  s_instances.push_back(this);
}

ThrustWindow::~ThrustWindow() 
{
  LOG(INFO) << "ThrustWindow Destructor [" << inspectable_web_contents() << "]";

  CloseImmediately();
  PlatformCleanUp();

  for(size_t i = 0; i < s_instances.size(); ++i) {
    if (s_instances[i] == this) {
      s_instances.erase(s_instances.begin() + i);
      break;
    }
  }
}

/******************************************************************************/
/* STATIC INTERFACE */
/******************************************************************************/
// static
ThrustWindow*
ThrustWindow::CreateNew(
    ThrustWindowBinding* binding,
    WebContents* web_contents,
    const gfx::Size& size,
    const std::string& title,
    const std::string& icon_path,
    const bool has_frame)
{
  ThrustWindow *browser = new ThrustWindow(binding, web_contents, size, 
                                           title, icon_path, 
                                           has_frame);
  return browser;
}

// static
ThrustWindow*
ThrustWindow::CreateNew(
    ThrustWindowBinding* binding,
    ThrustSession* session,
    const GURL& root_url,
    const gfx::Size& size,
    const std::string& title,
    const std::string& icon_path,
    const bool has_frame)
{
  LOG(INFO) << "ThrustWindow CreateNew";
  if(session == NULL) {
    session = ThrustShellBrowserClient::Get()->system_session();
  }
  WebContents::CreateParams create_params((BrowserContext*)session);
  WebContents* web_contents = WebContents::Create(create_params);
  
  NavigationController::LoadURLParams params(root_url);
  params.transition_type = PageTransitionFromInt(
      PAGE_TRANSITION_TYPED | PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents->GetController().LoadURLWithParams(params);

  LOG(INFO) << "ThrustWindow Constructor (web_contents created) [" 
            << web_contents << "]";

  return CreateNew(binding, web_contents, size, title, icon_path, 
                   has_frame);
}

// static
ThrustWindow* 
ThrustWindow::FromRenderView(
    int process_id, 
    int routing_id) 
{
  LOG(INFO) << "++++++++++++***********FROM RENDERVIEW: " << process_id << " " << routing_id;
  std::vector<ThrustWindow*> windows = ThrustWindow::instances();
  
  for(std::vector<ThrustWindow*>::iterator it = windows.begin(); 
      it != windows.end(); ++it) {
    ThrustWindow* w = *it;
    content::WebContents* web_contents = w->GetWebContents();
    int window_process_id = web_contents->GetRenderProcessHost()->GetID();
    int window_routing_id = web_contents->GetRoutingID();
    LOG(INFO) << "++++++++++++***********FROM RENDERVIEW: " << window_process_id << " " << window_routing_id;
    if(window_routing_id == routing_id && window_process_id == process_id) {
      return w;
    }
  }
  return NULL;
}


// static
void 
ThrustWindow::CloseAll() 
{
  std::vector<ThrustWindow*> open(s_instances);
  for (size_t i = 0; i < open.size(); ++i) {
    open[i]->Close();
  }
}

/******************************************************************************/
/* PUBLIC INTERFACE */
/******************************************************************************/
void 
ThrustWindow::SetTitle(
    const std::string& title)
{
  title_ = title;
  PlatformSetTitle(title);
}

void
ThrustWindow::Move(int x, int y)
{
  PlatformMove(x, y);
}

void
ThrustWindow::Resize(int width, int height)
{
  PlatformResize(width, height);
}

void
ThrustWindow::Close()
{
  content::WebContents* web_contents(GetWebContents());
  if(!web_contents) {
    CloseImmediately();
    return;
  }

  if(web_contents->NeedToFireBeforeUnload())
    web_contents->DispatchBeforeUnload(false);
  else
    web_contents->Close();
}

void 
ThrustWindow::OpenDevTools()
{
  inspectable_web_contents()->ShowDevTools();
}

void 
ThrustWindow::CloseDevTools()
{
  inspectable_web_contents()->CloseDevTools();
}

bool
ThrustWindow::IsDevToolsOpened()
{
  return inspectable_web_contents()->IsDevToolsViewShowing();
}

WebContents* 
ThrustWindow::GetWebContents() const {
  if (!inspectable_web_contents_)
    return NULL;
  return inspectable_web_contents()->GetWebContents();
}

/******************************************************************************/
/* WEBCONTENTSDELEGATE IMPLEMENTATION */
/******************************************************************************/
WebContents* 
ThrustWindow::OpenURLFromTab(
    WebContents* source,
    const content::OpenURLParams& params) 
{
  if(params.disposition != CURRENT_TAB)
    return NULL;

  content::NavigationController::LoadURLParams load_url_params(params.url);
  load_url_params.referrer = params.referrer;
  load_url_params.transition_type = params.transition;
  load_url_params.extra_headers = params.extra_headers;
  load_url_params.should_replace_current_entry =
      params.should_replace_current_entry;
  load_url_params.is_renderer_initiated = params.is_renderer_initiated;
  load_url_params.transferred_global_request_id =
      params.transferred_global_request_id;

  source->GetController().LoadURLWithParams(load_url_params);
  return source;
}


void 
ThrustWindow::RequestToLockMouse(
    WebContents* web_contents,
    bool user_gesture,
    bool last_unlocked_by_target) 
{
  /* Default implementation */
  web_contents->GotResponseToLockMouseRequest(true);
}

bool 
ThrustWindow::CanOverscrollContent() const 
{
  return false;
}

void 
ThrustWindow::CloseContents(
    WebContents* source) 
{
  /* The WebContents has closed so we start by destroying it. */
  DestroyWebContents();

  /* Once the WebContents is gone, we can close the window. The object itself */
  /* won't be freed, it will be freed when its binding is gone.               */
  CloseImmediately();
}


JavaScriptDialogManager* 
ThrustWindow::GetJavaScriptDialogManager() 
{
  if(!dialog_manager_) {
    dialog_manager_.reset(new ThrustShellJavaScriptDialogManager());
  }
  return dialog_manager_.get();
}

void 
ThrustWindow::ActivateContents(
    WebContents* contents) 
{
  LOG(INFO) << "Activate Content";
  GetWebContents()->GetRenderViewHost()->Focus();
}

void 
ThrustWindow::DeactivateContents(
    WebContents* contents) 
{
  LOG(INFO) << "Deactivate Content";
  GetWebContents()->GetRenderViewHost()->Blur();
}

void 
ThrustWindow::RendererUnresponsive(
    WebContents* source) 
{
  LOG(INFO) << "RendererUnresponsive";
  binding_->EmitUnresponsive();
}

void 
ThrustWindow::RendererResponsive(
    WebContents* source) 
{
  LOG(INFO) << "RendererResponsive";
  binding_->EmitResponsive();
}

void 
ThrustWindow::WorkerCrashed(
    WebContents* source) 
{
  LOG(INFO) << "WorkerCrashed";
  binding_->EmitWorkerCrashed();
}

ColorChooser* 
ThrustWindow::OpenColorChooser(
    WebContents* web_contents,
    SkColor color,
    const std::vector<ColorSuggestion>& suggestions)
{
  return chrome::ShowColorChooser(web_contents, color);
}

void 
ThrustWindow::RunFileChooser(
    WebContents* web_contents,
    const FileChooserParams& params)
{
  if(!web_dialog_helper_) {
    web_dialog_helper_.reset(new ThrustShellWebDialogHelper(this));
  }
  web_dialog_helper_->RunFileChooser(web_contents, params);
}

void 
ThrustWindow::EnumerateDirectory(
    WebContents* web_contents,
    int request_id,
    const base::FilePath& path)
{
  if(!web_dialog_helper_) {
    web_dialog_helper_.reset(new ThrustShellWebDialogHelper(this));
  }
  web_dialog_helper_->EnumerateDirectory(web_contents, request_id, path);
}

/******************************************************************************/
/* NOTIFICATIONOBSERFVER IMPLEMENTATION */
/******************************************************************************/
void 
ThrustWindow::Observe(
    int type,
    const NotificationSource& source,
    const NotificationDetails& details) 
{
  if (type == NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<NavigationEntry*, bool>* title =
        Details<std::pair<NavigationEntry*, bool>>(details).ptr();

    if (title->first) {
      std::string text = base::UTF16ToUTF8(title->first->GetTitle());
      SetTitle(text);
    }
  }
}

/******************************************************************************/
/* WEBCONTENTSOBSERVER IMPLEMENTATION */
/******************************************************************************/
bool 
ThrustWindow::OnMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ThrustWindow, message)
    IPC_MESSAGE_HANDLER(ThrustViewHostMsg_UpdateDraggableRegions,
                        PlatformUpdateDraggableRegions)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

bool 
ThrustWindow::OnMessageReceived(
    const IPC::Message& message,
    RenderFrameHost* render_frame_host)
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ThrustWindow, message)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_CreateWebViewGuest,
                        CreateWebViewGuest)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_DestroyWebViewGuest,
                        DestroyWebViewGuest)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestSetAutoSize,
                        WebViewGuestSetAutoSize)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestGo,
                        WebViewGuestGo)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestLoadUrl,
                        WebViewGuestLoadUrl)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestReload,
                        WebViewGuestReload)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestStop,
                        WebViewGuestStop)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestSetZoom,
                        WebViewGuestSetZoom)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestFind,
                        WebViewGuestFind)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestStopFinding,
                        WebViewGuestStopFinding)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestInsertCSS,
                        WebViewGuestInsertCSS)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestExecuteScript,
                        WebViewGuestExecuteScript)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestOpenDevTools,
                        WebViewGuestOpenDevTools)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestCloseDevTools,
                        WebViewGuestCloseDevTools)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestIsDevToolsOpened,
                        WebViewGuestIsDevToolsOpened)
    IPC_MESSAGE_HANDLER(ThrustFrameHostMsg_WebViewGuestJavaScriptDialogClosed,
                        WebViewGuestJavaScriptDialogClosed)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

/******************************************************************************/
/* WEBVIEWGUEST MESSAGE HANDLING */
/******************************************************************************/
void 
ThrustWindow::CreateWebViewGuest(
    const base::DictionaryValue& params,
    int* guest_instance_id)
{

  *guest_instance_id = 
    ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
        GetWebContents()->GetBrowserContext())->
      GetNextInstanceID();

  LOG(INFO) << "ThrustWindow CreateWebViewGuest " << *guest_instance_id;

  WebViewGuest* guest = WebViewGuest::Create(*guest_instance_id);

  GURL guest_site(base::StringPrintf("%s://webview",
                                     content::kGuestScheme));
  content::SiteInstance* guest_site_instance =
    content::SiteInstance::CreateForURL(
        GetWebContents()->GetBrowserContext(), guest_site);

  WebContents::CreateParams create_params(
      GetWebContents()->GetBrowserContext(),
      guest_site_instance);
  create_params.guest_delegate = guest;
  WebContents* guest_web_contents =
      WebContents::Create(create_params);

  guest->Init(guest_web_contents);
}

void 
ThrustWindow::DestroyWebViewGuest(
    int guest_instance_id)
{
  LOG(INFO) << "ThrustWindow DestroyWebViewGuest " << guest_instance_id;

  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->Destroy();
}


void 
ThrustWindow::WebViewEmit(
    int guest_instance_id,
    const std::string type,
    const base::DictionaryValue& params)
{
  /* We emit to the MainFrame as this is the only one that is authorized to */
  /* have <webview> tags.                                                   */
  GetWebContents()->GetMainFrame()->Send(
      new ThrustFrameMsg_WebViewEmit(
        GetWebContents()->GetMainFrame()->GetRoutingID(),
        guest_instance_id, type, params));
}

void 
ThrustWindow::WebViewGuestSetAutoSize(
    int guest_instance_id,
    const base::DictionaryValue& params)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  int min_width = 0;
  int min_height = 0;
  gfx::Size min_size;
  if(params.GetInteger("min_size.width", &min_width) &&
     params.GetInteger("min_size.height", &min_height)) {
     min_size = gfx::Size(min_width, min_height);
  }

  int max_width = 0;
  int max_height = 0;
  gfx::Size max_size;
  if(params.GetInteger("max_size.width", &max_width) &&
     params.GetInteger("max_size.height", &max_height)) {
     max_size = gfx::Size(max_width, max_height);
  }

  bool enabled = false;
  params.GetBoolean("enabled", &enabled);

  guest->SetAutoSize(enabled, min_size, max_size);
}
        
void 
ThrustWindow::WebViewGuestLoadUrl(
    int guest_instance_id,
    const std::string& url)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->LoadUrl(GURL(url));
}

void 
ThrustWindow::WebViewGuestGo(
    int guest_instance_id,
    int relative_index)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->Go(relative_index);
}

void 
ThrustWindow::WebViewGuestReload(
    int guest_instance_id,
    bool ignore_cache)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->Reload(ignore_cache);
}

void 
ThrustWindow::WebViewGuestStop(
    int guest_instance_id)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->Stop();
}

void 
ThrustWindow::WebViewGuestSetZoom(
    int guest_instance_id,
    double zoom_factor)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->SetZoom(zoom_factor);
}

void 
ThrustWindow::WebViewGuestFind(
    int guest_instance_id,
    int request_id,
    const std::string& search_text,
    const base::DictionaryValue& options)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  blink::WebFindOptions find_options;
  options.GetBoolean("forward", &find_options.forward);
  options.GetBoolean("match_case", &find_options.matchCase);
  options.GetBoolean("find_next", &find_options.findNext);
  options.GetBoolean("word_start", &find_options.wordStart);
  options.GetBoolean("medial_capital_as_word_start", 
                     &find_options.medialCapitalAsWordStart);

  guest->Find(request_id, search_text, find_options);
}

void 
ThrustWindow::WebViewGuestStopFinding(
    int guest_instance_id,
    const std::string& action)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  content::StopFindAction action_value = 
    content::STOP_FIND_ACTION_CLEAR_SELECTION;
  if(action.compare("clear") == 0) {
    action_value = content::STOP_FIND_ACTION_CLEAR_SELECTION;
  }
  if(action.compare("keep") == 0) {
    action_value = content::STOP_FIND_ACTION_KEEP_SELECTION;
  }
  if(action.compare("activate") == 0) {
    action_value = content::STOP_FIND_ACTION_ACTIVATE_SELECTION;
  }
  guest->StopFinding(action_value);
}

void 
ThrustWindow::WebViewGuestInsertCSS(
    int guest_instance_id,
    const std::string& css)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->InsertCSS(css);
}

void 
ThrustWindow::WebViewGuestExecuteScript(
    int guest_instance_id,
    const std::string& script)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->ExecuteScript(script);
}

void 
ThrustWindow::WebViewGuestOpenDevTools(
    int guest_instance_id)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->OpenDevTools();
}

void 
ThrustWindow::WebViewGuestCloseDevTools(
    int guest_instance_id)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->CloseDevTools();
}

void 
ThrustWindow::WebViewGuestIsDevToolsOpened(
    int guest_instance_id,
    bool* open)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  *open = guest->IsDevToolsOpened();
}

void 
ThrustWindow::WebViewGuestJavaScriptDialogClosed(
    int guest_instance_id,
    bool success, 
    const std::string& response)
{
  WebViewGuest* guest = 
    WebViewGuest::FromWebContents(
        ThrustShellBrowserClient::Get()->ThrustSessionForBrowserContext(
          GetWebContents()->GetBrowserContext())->
        GetGuestByInstanceID(guest_instance_id, 
          GetWebContents()->GetRenderProcessHost()->GetID()));

  guest->JavaScriptDialogClosed(success, response);
}

/******************************************************************************/
/* PROTECTED INTERFACE */
/******************************************************************************/
void
ThrustWindow::CloseImmediately()
{
  registrar_.RemoveAll();
  if(!is_closed_) {
    is_closed_ = true;
    PlatformCloseImmediately();
  }
}


/******************************************************************************/
/* PRIVATE INTERFACE */
/******************************************************************************/

void ThrustWindow::DestroyWebContents() {
  if(inspectable_web_contents_) {
    inspectable_web_contents_.reset();
  }
}

} // namespace thrust_shell
