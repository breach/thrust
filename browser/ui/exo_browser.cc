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
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/renderer_preferences.h"

#include "breach/common/breach_switches.h"
#include "breach/browser/breach_browser_main_parts.h"
#include "breach/browser/breach_content_browser_client.h"
#include "breach/browser/ui/dialog/breach_javascript_dialog_manager.h"
#include "breach/common/breach_messages.h"
#include "breach/browser/ui/exo_frame.h"

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
ExoBrowser::FrameForWebContents(
    WebContents* web_contents)
{
  std::map<std::string, ExoFrame*>::iterator it;
  for(it = frames_.begin; it != frames_.end(); ++it) {
    if((*it)->web_contents == web_contents) {
      return (it);
    }
  }
  return NULL;
}


ExoFrame*
ExoBrowser::AddFrame(
    ExoFrame* frame)
{
  frame->SetParent(this);
  frames_[frame->name()] = frame;
  PlatformAddFrame(frame);
}


void 
ExoBrowser::RemoveFrame(
    const std::string& name)
{
  std::map<std::string, ExoFrame*>::iterator it = frames_.find(name);
  if(it != frames_.end()) {
    (*it)->SetParent(NULL);
    PlatformRemoveFrame(*it);
    frames_.erase(it);
  }
}

void
ExoBrowser::Kill()
{
  while(frames_.begin() != frames_.end()) {
    RemoveFrame((*frames_.begin())->name());
  }
  PlatformKill();
  is_killed_ = true;
}

void 
ExoBrowser::LoadURLForFrame(
    const GURL& url, 
    const std::string& frame_name) 
{
  NodeThread::Get()->message_loop_proxy()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::LoadURLCallBack, wrapper_, url, frame_name));
}

WebContents* 
ExoBrowser::OpenURLFromTab(
    WebContents* source,
    const OpenURLParams& params) 
{
  ExoFrame* frame = FrameForWebContents(source);
  DCHECK(frame != NULL);
  /* TODO(spolu): Use params.transition            */
  /* TODO(spolu): Use params.referrer              */
  /* TODO(spolu): Use params.disposition           */
  if(frame) {
    NodeThread::Get()->message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::OpenURLCallBack, wrapper_, 
                   params.url.spec(), frame->name()));
  }
  return NULL;
}

void 
ExoBrowser::LoadingStateChanged(
    WebContents* source) 
{
  ExoFrame* frame = FrameForWebContents(source);
  DCHECK(frame != NULL);
  if(frame) {
    NodeThread::Get()->message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::LoadingStateChangeCallback, wrapper_, 
                   frame->name(), source->isLoading()));
  }
}

void 
ExoBrowser::RequestToLockMouse(
    WebContents* web_contents,
    bool user_gesture,
    bool last_unlocked_by_target) 
{
  /* Default implementation */
  web_contents->GotResponseToLockMouseRequest(true);
}

void 
ExoBrowser::CloseContents(
    WebContents* source) 
{
  ExoFrame* frame = FrameForWebContents(source);
  DCHECK(frame != NULL);
  if(frame) {
    NodeThread::Get()->message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::CloseFrameCallBack, wrapper_, 
                   frame->name()));
  }
}

void 
ExoBrowser::WebContentsCreated(
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
ExoBrowser::AddNewContents(
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
ExoBrowser::DidNavigateMainFramePostCommit(
    WebContents* web_contents) 
{
  ExoFrame* frame = FrameForWebContents(source);
  DCHECK(frame != NULL);
  if(frame) {
    NodeThread::Get()->message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::FrameNavigateCallBack, wrapper_, 
                   frame->name(),
                   frame->web_contents->GetURL()->spec()));
  }
}

JavaScriptDialogManager* 
ExoBrowser::GetJavaScriptDialogManager() 
{
  /* TODO(spolu): Eventually Move to API */
  if (!dialog_manager_)
    dialog_manager_.reset(new BreachJavaScriptDialogManager());
  return dialog_manager_.get();
}

void 
ExoBrowser::ActivateContents(
    WebContents* contents) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}

void 
ExoBrowser::DeactivateContents(
    WebContents* contents) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API (blur) */
}

void 
ExoBrowser::RendererUnresponsive(
    WebContents* source) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}

void 
ExoBrowser::WorkerCrashed(
    WebContents* source) 
{
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}




} // namespace breach
