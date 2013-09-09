// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "exo_browser/src/browser/ui/exo_browser.h"

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
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/common/renderer_preferences.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/browser/browser_main_parts.h"
#include "exo_browser/src/browser/content_browser_client.h"
#include "exo_browser/src/browser/ui/dialog/javascript_dialog_manager.h"
#include "exo_browser/src/common/messages.h"
#include "exo_browser/src/browser/ui/exo_frame.h"
#include "exo_browser/src/node/node_thread.h"
#include "exo_browser/src/node/api/exo_browser_wrap.h"

using namespace content;

namespace exo_browser {

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
  LOG(INFO) << "ExoBrowser Destructor";
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
  ExoBrowser* browser = new ExoBrowser(wrapper);
  browser->PlatformCreateWindow(size.width(), size.height());

  return browser;
}

void 
ExoBrowser::KillAll() 
{
  std::vector<ExoBrowser*> open(s_instances);
  for (size_t i = 0; i < open.size(); ++i) {
    open[i]->Kill();
  }
  base::MessageLoop::current()->RunUntilIdle();
}


ExoFrame* 
ExoBrowser::FrameForWebContents(
    const WebContents* web_contents)
{
  std::map<std::string, ExoFrame*>::iterator p_it;
  for(p_it = pages_.begin(); p_it != pages_.end(); ++p_it) {
    if((p_it->second)->web_contents_ == web_contents) {
      return (p_it->second);
    }
  }
  std::map<CONTROL_TYPE, ExoFrame*>::iterator c_it;
  for(c_it = controls_.begin(); c_it != controls_.end(); ++c_it) {
    if((c_it->second)->web_contents_ == web_contents) {
      return (c_it->second);
    }
  }
  return NULL;
}


void 
ExoBrowser::SetControl(
    CONTROL_TYPE type,
    ExoFrame* frame)
{
  std::map<CONTROL_TYPE, ExoFrame*>::iterator it = controls_.find(type);
  if(it == controls_.end()) {
    UnsetControl(type);
  }
  controls_[type] = frame;
  frame->SetType(ExoFrame::CONTROL_FRAME);
  frame->SetParent(this);
  PlatformSetControl(type, frame);
}

void
ExoBrowser::UnsetControl(
    CONTROL_TYPE type)
{
  std::map<CONTROL_TYPE, ExoFrame*>::iterator it = controls_.find(type);
  if(it != controls_.end()) {
    PlatformUnsetControl(it->first, it->second);
    (it->second)->SetType(ExoFrame::NOTYPE_FRAME);
    (it->second)->SetParent(NULL);
    controls_.erase(it);
  }
  /* Otherwise, nothing to do */
}

void
ExoBrowser::SetControlDimension(
    CONTROL_TYPE type,
    int size)
{
  PlatformSetControlDimension(type, size);
}


void
ExoBrowser::AddPage(
    ExoFrame* frame)
{
  frame->SetType(ExoFrame::PAGE_FRAME);
  frame->SetParent(this);
  pages_[frame->name()] = frame;
  PlatformAddPage(frame);
}


void 
ExoBrowser::RemovePage(
    const std::string& name)
{
  std::map<std::string, ExoFrame*>::iterator it = pages_.find(name);
  if(it != pages_.end()) {
    PlatformRemovePage(it->second);
    (it->second)->SetType(ExoFrame::NOTYPE_FRAME);
    (it->second)->SetParent(NULL);
    pages_.erase(it);
  }
  /* Otherwise, nothing to do */
}

void
ExoBrowser::ShowPage(
    const std::string& name)
{
  std::map<std::string, ExoFrame*>::iterator it = pages_.find(name);
  if(it != pages_.end()) {
    PlatformShowPage(it->second);
  }
  /* Otherwise, nothing to do */
}


void
ExoBrowser::RemoveFrame(
    const std::string& name)
{
  std::map<std::string, ExoFrame*>::iterator p_it;
  for(p_it = pages_.begin(); p_it != pages_.end(); ++p_it) {
    if((p_it->second)->name() == name) {
      return RemovePage((p_it->second)->name());
    }
  }
  std::map<CONTROL_TYPE, ExoFrame*>::iterator c_it;
  for(c_it = controls_.begin(); c_it != controls_.end(); ++c_it) {
    if((c_it->second)->name() == name) {
      return UnsetControl(c_it->first);
    }
  }

}


void
ExoBrowser::Kill()
{
  is_killed_ = true;
  while(pages_.begin() != pages_.end()) {
    RemovePage((pages_.begin()->second)->name());
  }
  while(controls_.begin() != controls_.end()) {
    UnsetControl(controls_.begin()->first);
  }
  PlatformKill();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoBrowserWrap::DispatchKill, wrapper_));
}

WebContents* 
ExoBrowser::OpenURLFromTab(
    WebContents* source,
    const OpenURLParams& params) 
{
  ExoFrame* frame = FrameForWebContents(source);
  if(frame) {
    /* Relevant header files:                              */
    /*  ui/base/window_open_disposition.h                  */
    /*  content/public/common/page_transition_types_list.h */

    /* TODO(spolu): Use params.transition            */
    /* TODO(spolu): Use params.referrer              */
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchOpenURL, wrapper_, 
                   params.url.spec(), params.disposition, frame->name()));
  }
  else {
    /* This is used when a newly created WebContents is not yet assigned to  */
    /* its fimal ExoFrame/ExoBrowser but needs a delegate to navigate to its */
    /* targeted delegate. See ExoBrowser::WebContentsCreated.                */
    source->GetController().LoadURL(
        params.url, params.referrer, params.transition, std::string());
  }
  return NULL;
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
  if(frame) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchFrameClose, wrapper_, 
                   frame->name()));
  }
}

bool 
ExoBrowser::PreHandleKeyboardEvent(
    WebContents* source,
    const NativeWebKeyboardEvent& event,
    bool* is_keyboard_shortcut)
{
  ExoFrame* frame = FrameForWebContents(source);
  if(frame) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchFrameKeyboard, wrapper_, 
                   frame->name(), event));
  }
  return false;
}

void 
ExoBrowser::HandleKeyboardEvent(
    WebContents* source,
    const NativeWebKeyboardEvent& event)
{
  //LOG(INFO) << "HandleKeyboardEvent " << event.windowsKeyCode;
}

void 
ExoBrowser::NavigationStateChanged(
    const WebContents* source,
    unsigned changed_flags)
{
  ExoFrame* frame = FrameForWebContents(source);
  if(frame) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchNavigationState, wrapper_, frame));
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
  LOG(INFO) << "WebContentsCreated: " << target_url 
            << "\nframe_name: " << frame_name
            << "\nsource_frame_id: " << source_frame_id
            << "\nsource_frame_id: " << source_frame_id
            << "\nnew_contents: " <<  new_contents;
  /* TODO(spolu): Call into API if necessary */

  /* We set this ExoBrowser as temporary WebContentsDelegate the            */
  /* OpenURLForTab method may need to be called for some WebContents, esp.  */
  /* when clicking on a link with `target="_blank"` and `rel="norerferrer"` */
  /* This delegate will get overriden when the new ExoFrame is later        */
  /* asynchronously added to an ExoBrowser.                                 */ 
  new_contents->SetDelegate(this);

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

  LOG(INFO) << "AddNewContents: " << (was_blocked ? *was_blocked : false)
            << "\nuser_gesture: " << user_gesture
            << "\ndisposition: " << disposition
            << "\nsource: " << source
            << "\nsource url: " << source->GetVisibleURL()
            << "\nnew_contents: " <<  new_contents
            << "\nnew_contents url: " <<  new_contents->GetVisibleURL()
            << "\nRenderProcessHost: " << new_contents->GetRenderProcessHost()
            << "\nRenderViewHost: " << new_contents->GetRenderViewHost() 
            << "\nView: " << new_contents->GetView()
            << "\nWaiting Response: " << new_contents->IsWaitingForResponse()
            << "\nInterstitial: " << new_contents->GetInterstitialPage();

  ExoFrame* src_frame = FrameForWebContents(source);
  DCHECK(src_frame != NULL);
  if(src_frame) {
    /* We generate a unique name for this new frame */
    std::ostringstream oss;
    static int pop_cnt = 0;
    oss << src_frame->name() << "-" << (++pop_cnt);

    ExoFrame* new_frame = new ExoFrame(oss.str(),
                                       new_contents);
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchFrameCreated, wrapper_, 
                   src_frame->name(), disposition, initial_pos, new_frame));
  }
}

JavaScriptDialogManager* 
ExoBrowser::GetJavaScriptDialogManager() 
{
  /* TODO(spolu): Eventually Move to API */
  if (!dialog_manager_)
    dialog_manager_.reset(new ExoBrowserJavaScriptDialogManager());
  return dialog_manager_.get();
}

void 
ExoBrowser::ActivateContents(
    WebContents* contents) 
{
  LOG(INFO) << "Activate Content";
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}

void 
ExoBrowser::DeactivateContents(
    WebContents* contents) 
{
  LOG(INFO) << "Dectivate Content";
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API (blur) */
}

void 
ExoBrowser::RendererUnresponsive(
    WebContents* source) 
{
  LOG(INFO) << "RendererUnresponsive";
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}

void 
ExoBrowser::WorkerCrashed(
    WebContents* source) 
{
  LOG(INFO) << "WorkerCrashed";
  /* TODO(spolu): find WebContents ExoFrame's name */
  /* TODO(spolu): Call into API */
}




} // namespace exo_browser
