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
#include "breach/browser/node/node_thread.h"
#include "breach/browser/node/api/exo_browser_wrap.h"

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
    WebContents* web_contents)
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
  while(pages_.begin() != pages_.end()) {
    RemovePage((pages_.begin()->second)->name());
  }
  while(controls_.begin() != controls_.end()) {
    UnsetControl(controls_.begin()->first);
  }
  PlatformKill();
  is_killed_ = true;

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
  DCHECK(frame != NULL);
  /* TODO(spolu): Use params.transition            */
  /* TODO(spolu): Use params.referrer              */
  /* TODO(spolu): Use params.disposition           */
  if(frame) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchOpenURL, wrapper_, 
                   params.url.spec(), frame->name()));
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
  DCHECK(frame != NULL);
  if(frame) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchFrameClose, wrapper_, 
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

bool 
ExoBrowser::PreHandleKeyboardEvent(
    WebContents* source,
    const NativeWebKeyboardEvent& event,
    bool* is_keyboard_shortcut)
{
  ExoFrame* frame = FrameForWebContents(source);
  DCHECK(frame != NULL);
  if(frame) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoBrowserWrap::DispatchFrameKeyboard, wrapper_, 
                   frame->name(), event));
  }
  return false;
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
