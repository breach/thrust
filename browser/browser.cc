// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/browser.h"

#include "base/auto_reset.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/devtools_manager.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/renderer_preferences.h"

#include "breach/common/breach_switches.h"
#include "breach/breach_browser_main_parts.h"
#include "breach/breach_content_browser_client.h"
#include "breach/browser/breach_devtools_frontend.h"
/* TODO(spolu): renaming post file creation */
#include "content/shell/common/shell_messages.h"
#include "content/shell/shell_javascript_dialog_manager.h"

using namespace content;

namespace breach {

const int Browser::kDefaultWindowWidthDip = 800;
const int Browser::kDefaultWindowHeightDip = 600;

std::vector<Browser*> Browser::windows_;
bool Browser::quit_message_loop_ = true;



class Browser::DevToolsWebContentsObserver : public WebContentsObserver {
 public:
  DevToolsWebContentsObserver(Browser* browser, 
                              WebContents* web_contents)
    : WebContentsObserver(web_contents),
      browser_(browser) 
  {
  }

  // WebContentsObserver
  virtual void 
  WebContentsDestroyed(WebContents* web_contents) OVERRIDE 
  {
    browser_->OnDevToolsWebContentsDestroyed();
  }

 private:
  Browser* browser_;

  DISALLOW_COPY_AND_ASSIGN(DevToolsWebContentsObserver);
};



Browser::Browser(WebContents* web_contents)
  : devtools_frontend_(NULL),
    is_fullscreen_(false),
    window_(NULL),
    url_edit_view_(NULL),
#if defined(OS_WIN) && !defined(USE_AURA)
    default_edit_wnd_proc_(0),
#endif
    headless_(false) 
{
  registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
      Source<WebContents>(web_contents));
  windows_.push_back(this);
}

Browser::~Browser() 
{
  PlatformCleanUp();

  for (size_t i = 0; i < windows_.size(); ++i) {
    if (windows_[i] == this) {
      windows_.erase(windows_.begin() + i);
      break;
    }
  }

  if (windows_.empty() && quit_message_loop_)
    base::MessageLoop::current()->PostTask(FROM_HERE,
                                           base::MessageLoop::QuitClosure());
}

Browser* 
Browser::CreateBrowser(WebContents* web_contents,
                       const gfx::Size& initial_size) 
{
  Browser* browser = new Browser(web_contents);
  browser->PlatformCreateWindow(initial_size.width(), initial_size.height());

  browser->web_contents_.reset(web_contents);
  web_contents->SetDelegate(browser);

  browser->PlatformSetContents();
  browser->PlatformResizeSubViews();

  return browser;
}

void 
Browser::CloseAllWindows() 
{
  base::AutoReset<bool> auto_reset(&quit_message_loop_, false);
  DevToolsManager::GetInstance()->CloseAllClientHosts();
  std::vector<Browser*> open_windows(windows_);
  for (size_t i = 0; i < open_windows.size(); ++i)
    open_windows[i]->Close();
  base::MessageLoop::current()->RunUntilIdle();
}

Browser* 
Browser::FromRenderViewHost(RenderViewHost* rvh) 
{
  for (size_t i = 0; i < windows_.size(); ++i) {
    if (windows_[i]->web_contents() &&
        windows_[i]->web_contents()->GetRenderViewHost() == rvh) {
      return windows_[i];
    }
  }
  return NULL;
}

// static
void 
Browser::Initialize() 
{
  PlatformInitialize(
      gfx::Size(kDefaultWindowWidthDip, kDefaultWindowHeightDip));
}

Browser* Browser::CreateNewWindow(BrowserContext* browser_context,
                                  const GURL& url,
                                  SiteInstance* site_instance,
                                  int routing_id,
                                  const gfx::Size& initial_size) 
{
  WebContents::CreateParams create_params(browser_context, site_instance);
  create_params.routing_id = routing_id;
  if (!initial_size.IsEmpty())
    create_params.initial_size = initial_size;
  else
    create_params.initial_size =
        gfx::Size(kDefaultWindowWidthDip, kDefaultWindowHeightDip);
  WebContents* web_contents = WebContents::Create(create_params);
  Browser* browser = CreateBrowser(web_contents, create_params.initial_size);
  if (!url.is_empty())
    browser->LoadURL(url);
  return browser;
}

void 
Browser::LoadURL(const GURL& url) 
{
  LoadURLForFrame(url, std::string());
}

void 
Browser::LoadURLForFrame(const GURL& url, 
                         const std::string& frame_name) 
{
  NavigationController::LoadURLParams params(url);
  params.transition_type = PageTransitionFromInt(
      PAGE_TRANSITION_TYPED | PAGE_TRANSITION_FROM_ADDRESS_BAR);
  params.frame_name = frame_name;
  web_contents_->GetController().LoadURLWithParams(params);
  web_contents_->GetView()->Focus();
}

void 
Browser::GoBackOrForward(int offset) 
{
  web_contents_->GetController().GoToOffset(offset);
  web_contents_->GetView()->Focus();
}

void 
Browser::Reload() 
{
  web_contents_->GetController().Reload(false);
  web_contents_->GetView()->Focus();
}

void 
Browser::Stop() 
{
  web_contents_->Stop();
  web_contents_->GetView()->Focus();
}

void 
Browser::UpdateNavigationControls() 
{
  int current_index = web_contents_->GetController().GetCurrentEntryIndex();
  int max_index = web_contents_->GetController().GetEntryCount() - 1;

  PlatformEnableUIControl(BACK_BUTTON, current_index > 0);
  PlatformEnableUIControl(FORWARD_BUTTON, current_index < max_index);
  PlatformEnableUIControl(STOP_BUTTON, web_contents_->IsLoading());
}

void 
Browser::ShowDevTools() 
{
  if (devtools_frontend_) {
    devtools_frontend_->Focus();
    return;
  }
  devtools_frontend_ = BreachDevToolsFrontend::Show(web_contents());
  devtools_observer_.reset(new DevToolsWebContentsObserver(
      this, devtools_frontend_->frontend_shell()->web_contents()));
}

void 
Browser::CloseDevTools() 
{
  if (!devtools_frontend_)
    return;
  devtools_observer_.reset();
  devtools_frontend_->Close();
  devtools_frontend_ = NULL;
}

gfx::NativeView 
Browser::GetContentView() 
{
  if (!web_contents_)
    return NULL;
  return web_contents_->GetView()->GetNativeView();
}

WebContents* 
Browser::OpenURLFromTab(WebContents* source,
                        const OpenURLParams& params) 
{
  // The only one we implement for now.
  DCHECK(params.disposition == CURRENT_TAB);
  source->GetController().LoadURL(
      params.url, params.referrer, params.transition, std::string());
  return source;
}

void 
Browser::LoadingStateChanged(WebContents* source) 
{
  UpdateNavigationControls();
  PlatformSetIsLoading(source->IsLoading());
}

void 
Browser::ToggleFullscreenModeForTab(WebContents* web_contents,
                                    bool enter_fullscreen) 
{
  if (is_fullscreen_ != enter_fullscreen) {
    is_fullscreen_ = enter_fullscreen;
    web_contents->GetRenderViewHost()->WasResized();
  }
}

bool 
Browser::IsFullscreenForTabOrPending(const WebContents* web_contents) const 
{
  return is_fullscreen_;
}

void 
Browser::RequestToLockMouse(WebContents* web_contents,
                            bool user_gesture,
                            bool last_unlocked_by_target) 
{
  web_contents->GotResponseToLockMouseRequest(true);
}

void 
Browser::CloseContents(WebContents* source) 
{
  Close();
}

bool 
Browser::CanOverscrollContent() const 
{
  return false;
}

void 
Browser::WebContentsCreated(WebContents* source_contents,
                          int64 source_frame_id,
                          const string16& frame_name,
                          const GURL& target_url,
                          WebContents* new_contents) 
{
  CreateBrowser(new_contents, source_contents->GetView()->GetContainerSize());
}

void 
Browser::DidNavigateMainFramePostCommit(WebContents* web_contents) 
{
  PlatformSetAddressBarURL(web_contents->GetURL());
}

JavaScriptDialogManager* 
Browser::GetJavaScriptDialogManager() 
{
  /* TODO(spolu): renaming post file creation */
  if (!dialog_manager_)
    dialog_manager_.reset(new ShellJavaScriptDialogManager());
  return dialog_manager_.get();
}

bool 
Browser::AddMessageToConsole(WebContents* source,
                             int32 level,
                             const string16& message,
                             int32 line_no,
                             const string16& source_id) 
{
  return false;
}

void 
Browser::RendererUnresponsive(WebContents* source) 
{
  return;
}

void 
Browser::ActivateContents(WebContents* contents) 
{
  contents->GetRenderViewHost()->Focus();
}

void 
Browser::DeactivateContents(WebContents* contents) 
{
  contents->GetRenderViewHost()->Blur();
}

void 
Browser::WorkerCrashed(WebContents* source) 
{
  return;
}

void 
Browser::Observe(int type,
               const NotificationSource& source,
               const NotificationDetails& details) 
{
  if (type == NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<NavigationEntry*, bool>* title =
        Details<std::pair<NavigationEntry*, bool> >(details).ptr();

    if (title->first) {
      string16 text = title->first->GetTitle();
      PlatformSetTitle(text);
    }
  } else {
    NOTREACHED();
  }
}

void 
Browser::OnDevToolsWebContentsDestroyed() 
{
  devtools_observer_.reset();
  devtools_frontend_ = NULL;
}

} // namespace breach
