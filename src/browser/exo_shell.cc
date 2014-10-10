// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/exo_shell.h"

#include "base/auto_reset.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "ui/gfx/codec/png_codec.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/browser/favicon_status.h"

#include "src/common/switches.h"
#include "src/browser/browser_main_parts.h"
#include "src/browser/browser_client.h"
#include "src/browser/dialog/javascript_dialog_manager.h"
#include "src/browser/dialog/file_select_helper.h"
#include "src/common/messages.h"
#include "src/browser/ui/views/menu_bar.h"
#include "src/browser/ui/views/menu_layout.h"

#if defined(USE_X11)
#include "src/browser/ui/views/global_menu_bar_x11.h"
#elif defined(OS_WIN)
#endif

using namespace content;

namespace exo_shell {

std::vector<ExoShell*> ExoShell::s_instances;


ExoShell::ExoShell(
    WebContents* web_contents,
    const gfx::Size& size,
    const std::string& title,
    const std::string& icon_path,
    const bool has_frame)
  : WebContentsObserver(web_contents),
    is_closed_(false),
    title_(title),
    has_frame_(has_frame),
    inspectable_web_contents_(
        brightray::InspectableWebContents::Create(web_contents))
{
  web_contents->SetDelegate(this);
  inspectable_web_contents()->SetDelegate(this);
  WebContentsObserver::Observe(web_contents);

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
    scoped_ptr<SkBitmap> decoded(new SkBitmap());
    gfx::PNGCodec::Decode(data, file_contents.length(), decoded.get());
    if(!decoded->empty()) {
      icon_ = gfx::Image::CreateFrom1xBitmap(*decoded.release());
    }
    else {
      icon_ = gfx::Image();
    }
  }
  base::ThreadRestrictions::SetIOAllowed(false);

  /*
  renderer_preferences_util::UpdateFromSystemSettings(
      web_contents_->GetMutableRendererPrefs());
  web_contents_->GetRenderViewHost()->SyncRendererPrefs();
   */
  PlatformCreateWindow(size);


  LOG(INFO) << "ExoShell Constructor [" << web_contents << "]";
  s_instances.push_back(this);
}

ExoShell::~ExoShell() 
{
  LOG(INFO) << "ExoShell Destructor [" << inspectable_web_contents() << "]";

  Close();
  PlatformCleanUp();

  for (size_t i = 0; i < s_instances.size(); ++i) {
    if (s_instances[i] == this) {
      s_instances.erase(s_instances.begin() + i);
      break;
    }
  }
}


ExoShell*
ExoShell::CreateNew(
    WebContents* web_contents,
    const gfx::Size& size,
    const std::string& title,
    const std::string& icon_path,
    const bool has_frame)
{
  ExoShell *browser = new ExoShell(web_contents, size, 
                                   title, icon_path, has_frame);
  return browser;
}

ExoShell*
ExoShell::CreateNew(
    ExoSession* session,
    const GURL& root_url,
    const gfx::Size& size,
    const std::string& title,
    const std::string& icon_path,
    const bool has_frame)
{
  if(session == NULL) {
    session = ExoShellBrowserClient::Get()->system_session();
  }
  WebContents::CreateParams create_params((BrowserContext*)session);
  WebContents* web_contents = WebContents::Create(create_params);
  
  NavigationController::LoadURLParams params(root_url);
  params.transition_type = PageTransitionFromInt(
      PAGE_TRANSITION_TYPED | PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents->GetController().LoadURLWithParams(params);

  LOG(INFO) << "ExoShell Constructor (web_contents created) [" 
            << web_contents << "]";

  return CreateNew(web_contents, size, title, icon_path, has_frame);
}

WebContents* 
ExoShell::GetWebContents() const {
  if (!inspectable_web_contents_)
    return NULL;
  return inspectable_web_contents()->GetWebContents();
}

void 
ExoShell::CloseAll() 
{
  std::vector<ExoShell*> open(s_instances);
  for (size_t i = 0; i < open.size(); ++i) {
    open[i]->Close();
  }
}

void 
ExoShell::SetTitle(
    const std::string& title)
{
  title_ = title;
  PlatformSetTitle(title);
}

void
ExoShell::Close()
{
  registrar_.RemoveAll();
  if(!is_closed_) {
    is_closed_ = true;
    PlatformClose();
  }
}

void
ExoShell::Move(int x, int y)
{
	PlatformMove(x, y);
}

void
ExoShell::Resize(int width, int height)
{
	PlatformResize(width, height);
}

WebContents* 
ExoShell::OpenURLFromTab(
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
ExoShell::RequestToLockMouse(
    WebContents* web_contents,
    bool user_gesture,
    bool last_unlocked_by_target) 
{
  /* Default implementation */
  web_contents->GotResponseToLockMouseRequest(true);
}

bool 
ExoShell::CanOverscrollContent() const 
{
  return false;
}

void 
ExoShell::CloseContents(
    WebContents* source) 
{
  if(inspectable_web_contents_) {
    inspectable_web_contents_.reset();
  }
  Close();
}


JavaScriptDialogManager* 
ExoShell::GetJavaScriptDialogManager() 
{
  /* TODO(spolu): Eventually Move to API */
  if (!dialog_manager_)
    dialog_manager_.reset(new ExoShellJavaScriptDialogManager());
  return dialog_manager_.get();
}

void 
ExoShell::ActivateContents(
    WebContents* contents) 
{
  LOG(INFO) << "Activate Content";
  /* TODO(spolu): Call into Platform */
}

void 
ExoShell::DeactivateContents(
    WebContents* contents) 
{
  LOG(INFO) << "Deactivate Content";
  /* TODO(spolu): Call into Platform */
}

void 
ExoShell::RendererUnresponsive(
    WebContents* source) 
{
  LOG(INFO) << "RendererUnresponsive";
  /* TODO(spolu): Notify */
}

void 
ExoShell::RendererResponsive(
    WebContents* source) 
{
  LOG(INFO) << "RendererResponsive";
  /* TODO(spolu): Notify */
}

void 
ExoShell::WorkerCrashed(
    WebContents* source) 
{
  LOG(INFO) << "WorkerCrashed";
  /* TODO(spolu): Notify */
}

void 
ExoShell::RunFileChooser(
    WebContents* web_contents,
    const FileChooserParams& params)
{
  //FileSelectHelper::RunFileChooser(web_contents, params);
}

void 
ExoShell::EnumerateDirectory(
    WebContents* web_contents,
    int request_id,
    const base::FilePath& path)
{
  //FileSelectHelper::EnumerateDirectory(web_contents, request_id, path);
}

void 
ExoShell::Observe(
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

bool 
ExoShell::OnMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ExoShell, message)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

} // namespace exo_shell
