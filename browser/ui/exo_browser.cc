// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "breach/browser/ui/exo_browser.h"

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
#include "breach/browser/breach_browser_main_parts.h"
#include "breach/browser/breach_content_browser_client.h"
#include "breach/browser/devtools/breach_devtools_frontend.h"
#include "breach/browser/ui/dialog/breach_javascript_dialog_manager.h"
#include "breach/common/breach_messages.h"

using namespace content;

namespace breach {

const int ExoBrowser::kDefaultWindowWidth = 800;
const int ExoBrowser::kDefaultWindowHeight = 600;

std::vector<ExoBrowser*> ExoBrowser::s_instances;


ExoBrowser::ExoBrowser(
    ExoBrowserWrap* wrapper)
  : window_(NULL),
#if defined(OS_WIN) && !defined(USE_AURA)
    default_edit_wnd_proc_(0),
#endif
    wrapper_(wrapper),
    is_killed_(false)
{
  registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
      Source<WebContents>(web_contents));
  s_instances.push_back(this);
}

ExoBrowser::~ExoBrowser() 
{
  PlatformCleanUp();

  for (size_t i = 0; i < s_instances.size(); ++i) {
    if (s_instances[i] == this) {
      s_instances.erase(s_instances.begin() + i);
      break;
    }
  }
}


void 
ExoBrowser::Initialize() 
{
  PlatformInitialize(
      gfx::Size(kDefaultWindowWidth, kDefaultWindowHeight));
}


ExoBrowser*
ExoBrowser::CreateNew(
    ExoBrowserWrap* wrapper,
    const gfx::Size& size)
{
  ExoBrowser* browser = new ExoBrowser(wrapper)
  browser->PlatformCreateWindow(size.width(), size.height());

  return browser;
}

void 
ExoBrowser::KillAll() 
{
  std::vector<ExoBrowser*> open(s_instances);
  for (size_t i = 0; i < open.size(); ++i)
    open[i]->Kill();
  base::MessageLoop::current()->RunUntilIdle();
}


ExoFrame* 
ExoBrowser::NewFrame(
    const std::string& name,
    const gfx::Point& position
    const gfx::Size& size,
    const GURL& url)
{
  //ExoFrame* frame = new ExoFrame(name, this, 
}

ExoFrame*
ExoBrowser::AddFrame(
    ExoFrame* frame)
{
}


void 
ExoBrowser::KillFrame(
    const std::string& name)
{
}

void
ExoBrowser::Kill()
{
}

void 
Browser::LoadURLForFrame(
    const GURL& url, 
    const std::string& frame_name) 
{
  NodeThread::Get()->message_loop_proxy()->PostTask(
      FROM_HERE,
      base::Bind(&BrowserWrap::LoadURLCallBack, wrapper_, url, frame_name));
}
WebContents* 
Browser::OpenURLFromTab(
    WebContents* source,
    const OpenURLParams& params) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Use params.transition            */
  /* TODO(spolu): Use params.referrer              */
  /* TODO(spolu): Use params.disposition           */
  std::string source_frame();
  NodeThread::Get()->message_loop_proxy()->PostTask(
      FROM_HERE,
      base::Bind(&BrowserWrap::OpenURLCallBack, wrapper_, 
                 params.url, source_frame));
}

void 
Browser::LoadingStateChanged(
    WebContents* source) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API   */
  /* Can use: source->IsLoading() */
}

void 
Browser::RequestToLockMouse(
    WebContents* web_contents,
    bool user_gesture,
    bool last_unlocked_by_target) 
{
  /* Default implementation */
  web_contents->GotResponseToLockMouseRequest(true);
}

void 
Browser::CloseContents(
    WebContents* source) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  std::string frame_name();
  NodeThread::Get()->message_loop_proxy()->PostTask(
      FROM_HERE,
      base::Bind(&BrowserWrap::CloseFrameCallBack, wrapper_, frame_name));
}

void 
Browser::WebContentsCreated(
    WebContents* source_contents,
    int64 source_frame_id,
    const string16& frame_name,
    const GURL& target_url,
    WebContents* new_contents) 
{
  LOG(INFO) << "WebContentsCreated";
  /* TODO(spolu): Call into API if necessary */
}

void 
Browser::AddNewContents(
    WebContents* source,
    WebContents* new_contents,
    WindowOpenDisposition disposition,
    const gfx::Rect& initial_pos,
    bool user_gesture,
    bool* was_blocked) 
{
  LOG(INFO) << "AddNewContents";
  /* TODO(spolu): Call into API */
}

void 
Browser::DidNavigateMainFramePostCommit(
    WebContents* web_contents) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}

JavaScriptDialogManager* 
Browser::GetJavaScriptDialogManager() 
{
  /* TODO(spolu): Eventually Move to API */
  if (!dialog_manager_)
    dialog_manager_.reset(new BreachJavaScriptDialogManager());
  return dialog_manager_.get();
}

void 
Browser::ActivateContents(
    WebContents* contents) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}

void 
Browser::DeactivateContents(
    WebContents* contents) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API (blur) */
}

void 
Browser::RendererUnresponsive(
    WebContents* source) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}

void 
Browser::WorkerCrashed(
    WebContents* source) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}


void 
ExoBrowser::Observe(
    int type,
    const NotificationSource& source,
    const NotificationDetails& details) 
{
  if (type == NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<NavigationEntry*, bool>* title =
        Details<std::pair<NavigationEntry*, bool> >(details).ptr();

    if (title->first) {
      /* TODO(spolu): Call into JS */
      //string16 text = title->first->GetTitle();
      //PlatformSetTitle(text);
    }
  } else {
    NOTREACHED();
  }
}



} // namespace breach
