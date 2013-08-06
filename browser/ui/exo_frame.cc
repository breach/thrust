// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "breach/browser/ui/exo_frame.h"

#include "breach/browser/ui/exo_browser.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"

using namespace content;

namespace breach {

ExoFrame::ExoFrame(
    std::string& name,
    content::WebContents* web_contents,
    ExoFrameWrap* wrapper)
  : name_(name),
    wrapper_(wrapper)
{
  web_contents_.reset(web_contents);
}

ExoFrame::ExoFrame(
    std::string& name,
    ExoFrameWrap* wrapper)
  : name_(name),
    wrapper_(wrapper)
{
  WebContents::CreateParams create_params(
      BreachContentBrowserClient::Get()->browser_context());
  WebContents* web_contents = WebContents::Create(create_params);
  web_contents_.reset(web_conents);
}

void
ExoFrame::SetParent(
    ExoBrowser* parent)
{
  parent_ = parent;
  web_contents_->SetDelegate(parent_);
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
  params.frame_name = name;
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
ExoFrame::SetSize(
    gfx::Size& size)
{
  PlatformSetSize(size);
}

void 
ExoFrame::SetPosition(
    gfx::Point& position)
{
  PlatformSetPosition(position);
}

} // namespace breach

