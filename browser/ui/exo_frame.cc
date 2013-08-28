// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "breach/browser/ui/exo_frame.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/common/favicon_url.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h" 
#include "breach/browser/ui/exo_browser.h"
#include "breach/browser/node/node_thread.h"
#include "breach/browser/breach_content_browser_client.h"
#include "breach/browser/node/api/exo_frame_wrap.h"

using namespace content;

namespace breach {

ExoFrame::ExoFrame(
    const std::string& name,
    content::WebContents* web_contents,
    ExoFrameWrap* wrapper)
  : WebContentsObserver(web_contents),
    name_(name),
    type_(NOTYPE_FRAME),
    wrapper_(wrapper)
{
  web_contents_.reset(web_contents);
  WebContentsObserver::Observe(web_contents);
  LOG(INFO) << "ExoFrame Constructor [" << web_contents << "]";
}

ExoFrame::ExoFrame(
    const std::string& name,
    ExoFrameWrap* wrapper)
  : name_(name),
    type_(NOTYPE_FRAME),
    wrapper_(wrapper)
{
  WebContents::CreateParams create_params(
      (BrowserContext*)BreachContentBrowserClient::Get()->browser_context());
  WebContents* web_contents = WebContents::Create(create_params);
  web_contents_.reset(web_contents);
  WebContentsObserver::Observe(web_contents);
}

void
ExoFrame::SetParent(
    ExoBrowser* parent)
{
  LOG(INFO) << "ExoFrame SetParent [" << web_contents_ << "]: " << parent;
  parent_ = parent;
  web_contents_->SetDelegate(parent_);
}

void
ExoFrame::SetType(
    FRAME_TYPE type)
{
  type_ = type;
}


ExoFrame::~ExoFrame()
{
  /* Not much to do. If we're here that means that our JS wrapper has been */
  /* reclaimed and that most of what is related to use has been destroyed  */
  /* already. Our associated web_contents, should be deleted with its      */
  /* scoped_ptr.                                                           */
  LOG(INFO) << "ExoFrame Destructor [" << web_contents_ << "]";
}

void
ExoFrame::LoadURL(
    const GURL& url)
{
  NavigationController::LoadURLParams params(url);
  /* TODO(spolu): Manage `transition_type` */
  params.transition_type = PageTransitionFromInt(
      PAGE_TRANSITION_TYPED | PAGE_TRANSITION_FROM_ADDRESS_BAR);
  web_contents_->GetController().LoadURLWithParams(params);
}

void
ExoFrame::GoBackOrForward(int offset)
{
  web_contents_->GetController().GoToOffset(offset);
}

void 
ExoFrame::Reload() 
{
  web_contents_->GetController().Reload(true);
}

void 
ExoFrame::Stop() 
{
  web_contents_->Stop();
}

void
ExoFrame::Focus()
{
  web_contents_->GetView()->Focus();
}

void
ExoFrame::Find(
    int request_id, 
    const string16& search_text,
    const WebKit::WebFindOptions& options)
{
  web_contents_->GetRenderViewHost()->Find(request_id, search_text, options);
}

void
ExoFrame::StopFinding(
    StopFindAction action)
{
  web_contents_->GetRenderViewHost()->StopFinding(action);
}

/******************************************************************************/
/*                    WEBCONTENTSOBSERVER IMPLEMENTATION                      */
/******************************************************************************/
void 
ExoFrame::DidUpdateFaviconURL(
    int32 page_id,
    const std::vector<FaviconURL>& candidates)
{
  if(wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchFaviconUpdate, wrapper_, candidates));
  }
}

void 
ExoFrame::DidFailLoad(
    int64 frame_id,
    const GURL& validated_url,
    bool is_main_frame,
    int error_code,
    const string16& error_description,
    RenderViewHost* render_view_host)
{
  if(is_main_frame && wrapper_) {
    std::string desc = UTF16ToUTF8(error_description);
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchLoadFail, wrapper_,
                   validated_url.spec(), error_code, desc));
  }
}

void 
ExoFrame::DidFinishLoad(
    int64 frame_id,
    const GURL& validated_url,
    bool is_main_frame,
    RenderViewHost* render_view_host)
{
  if(is_main_frame && wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchLoadFinish, wrapper_,
                   validated_url.spec()));
    web_contents_->GetView()->Focus();
  }
}

void 
ExoFrame::DidStartLoading(
    RenderViewHost* render_view_host)
{
  if(wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchLoadingStart, wrapper_));
  }
}

void 
ExoFrame::DidStopLoading(
    RenderViewHost* render_view_host)
{
  if(wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchLoadingStop, wrapper_));
  }
}


void 
ExoFrame::Observe(
    int type,
    const NotificationSource& source,
    const NotificationDetails& details) 
{
  NOTREACHED();
}


} // namespace breach

