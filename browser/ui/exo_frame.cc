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
  registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
      Source<WebContents>(web_contents));

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
  registrar_.Add(this, NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
      Source<WebContents>(web_contents));
  WebContentsObserver::Observe(web_contents);
}

void
ExoFrame::SetParent(
    ExoBrowser* parent)
{
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
  web_contents_->GetView()->Focus();
}

void
ExoFrame::GoBackOrForward(int offset)
{
  web_contents_->GetController().GoToOffset(offset);
  web_contents_->GetView()->Focus();
}

void 
ExoFrame::Reload() 
{
  web_contents_->GetController().Reload(false);
  web_contents_->GetView()->Focus();
}

void 
ExoFrame::Stop() 
{
  web_contents_->Stop();
  web_contents_->GetView()->Focus();
}

void
ExoFrame::Focus()
{
  web_contents_->GetView()->Focus();
}

/******************************************************************************/
/*                    WEBCONTENTSOBSERVER IMPLEMENTATION                      */
/******************************************************************************/

void 
ExoFrame::DidUpdateFaviconURL(
    int32 page_id,
    const std::vector<FaviconURL>& candidates)
{
  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::DispatchFaviconUpdate, wrapper_, candidates));
}

void 
ExoFrame::ProvisionalChangeToMainFrameUrl(
    const GURL& url,
    content::RenderViewHost* render_view_host)
{
  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoFrameWrap::DispatchPendingURL, wrapper_, url.spec()));
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
  if(is_main_frame) {
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
  if(is_main_frame) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchLoadFinish, wrapper_,
                   validated_url.spec()));
  }
}

void 
ExoFrame::DidStartLoading(
    RenderViewHost* render_view_host)
{
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchLoadingStart, wrapper_));
}

void 
ExoFrame::DidStopLoading(
    RenderViewHost* render_view_host)
{
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::DispatchLoadingStop, wrapper_));
}


void 
ExoFrame::Observe(
    int type,
    const NotificationSource& source,
    const NotificationDetails& details) 
{
  if (type == NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<NavigationEntry*, bool>* title =
        Details<std::pair<NavigationEntry*, bool> >(details).ptr();

    if (title->first) {
      std::string t = UTF16ToUTF8(title->first->GetTitle());
      NodeThread::Get()->PostTask(
          FROM_HERE,
          base::Bind(&ExoFrameWrap::DispatchTitleUpdate, wrapper_, t));
    }
  } else {
    NOTREACHED();
  }
}


} // namespace breach

