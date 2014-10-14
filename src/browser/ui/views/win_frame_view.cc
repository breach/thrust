// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2014 GitHub, Inc.
// See the LICENSE file.

#include "src/browser/ui/views/win_frame_view.h"

#include "ui/gfx/win/dpi.h"
#include "ui/views/widget/widget.h"
#include "ui/views/win/hwnd_util.h"

#include "src/browser/thrust_window.h"

namespace thrust_shell {

namespace {

const char kViewClassName[] = "WinFrameView";

}  // namespace


WinFrameView::WinFrameView() 
{
}

WinFrameView::~WinFrameView() 
{
}


gfx::Rect 
WinFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const 
{
  gfx::Size size(client_bounds.size());
  ClientAreaSizeToWindowSize(&size);
  return gfx::Rect(client_bounds.origin(), size);
}

int 
WinFrameView::NonClientHitTest(
    const gfx::Point& point) 
{
  if (shell_->has_frame())
    return frame_->client_view()->NonClientHitTest(point);
  else
    return FramelessView::NonClientHitTest(point);
}

gfx::Size 
WinFrameView::GetMinimumSize() const 
{
  gfx::Size size = FramelessView::GetMinimumSize();
  return gfx::win::DIPToScreenSize(size);
}

gfx::Size 
WinFrameView::GetMaximumSize() const 
{
  gfx::Size size = FramelessView::GetMaximumSize();
  return gfx::win::DIPToScreenSize(size);
}

const char* 
WinFrameView::GetClassName() const 
{
  return kViewClassName;
}

void 
WinFrameView::ClientAreaSizeToWindowSize(
    gfx::Size* size) const 
{
  // AdjustWindowRect seems to return a wrong window size.
  gfx::Size window = frame_->GetWindowBoundsInScreen().size();
  gfx::Size client = frame_->GetClientAreaBoundsInScreen().size();
  size->set_width(size->width() + window.width() - client.width());
  size->set_height(size->height() + window.height() - client.height());
}

}  // namespace thrust_shell
