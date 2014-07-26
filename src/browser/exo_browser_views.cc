// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/exo_browser.h"

#if defined(OS_WIN)
#include <shobjidl.h>
#endif

#if defined(USE_X11)
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xlib.h>
#endif

#include <string>
#include <vector>

#include "base/file_util.h"
#include "base/threading/thread_restrictions.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/views/background.h"
#include "ui/views/controls/webview/unhandled_keyboard_event_handler.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/window/client_view.h"
#include "ui/views/widget/widget.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#include "ui/views/background.h"
#include "ui/views/controls/webview/unhandled_keyboard_event_handler.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/window/client_view.h"
#include "ui/views/widget/widget.h"
#include "ui/views/layout/fill_layout.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "vendor/brightray/browser/inspectable_web_contents_view.h"

#if defined(USE_X11)
#include "src/browser/ui/views/frameless_view.h"
#include "ui/gfx/x/x11_types.h"
#include "ui/views/window/native_frame_view.h"
#elif defined(OS_WIN)
#include "src/browser/ui/views/win_frame_view.h"
#include "base/win/scoped_comptr.h"
#endif

using namespace content;

namespace exo_browser {

namespace {

class ExoBrowserClientView : public views::ClientView {
 public:
  ExoBrowserClientView(
      views::Widget* widget,
      ExoBrowser* contents_view)
      : views::ClientView(widget, contents_view) 
  {
  }
  virtual ~ExoBrowserClientView() {}

  virtual bool 
  CanClose() OVERRIDE 
  {
    static_cast<ExoBrowser*>(contents_view())->Close();
    return false;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ExoBrowserClientView);
};

}  // namespace

void 
ExoBrowser::PlatformCleanUp() 
{
  window_->RemoveObserver(this);
}

void 
ExoBrowser::PlatformCreateWindow(
    const gfx::Size& size)
{
  window_.reset(new views::Widget());

  LOG(INFO) << "Create Window: " << size.width() << "x" << size.height();

  gfx::Rect bounds(0, 0, size.width(), size.height());
  window_->AddObserver(this);

  views::Widget::InitParams params;
  params.ownership = views::Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET;
  params.bounds = bounds;
  params.delegate = this;
  params.type = views::Widget::InitParams::TYPE_WINDOW;
  params.top_level = true;
  params.remove_standard_frame = !has_frame_;

  window_->Init(params);

  // Add web view.
  SetLayoutManager(new views::FillLayout());
  set_background(views::Background::CreateStandardPanelBackground());
  AddChildView(inspectable_web_contents()->GetView()->GetView());

  window_->CenterWindow(bounds.size());
  Layout();
  window_->Show();
}

void 
ExoBrowser::PlatformClose() 
{
  window_->Close();
}

void 
ExoBrowser::PlatformSetTitle(
    const std::string& title) 
{
  window_->UpdateWindowTitle();
}

void
ExoBrowser::PlatformFocus(bool focus)
{
  if (focus)
    window_->Activate();
  else
    window_->Deactivate();
}

void
ExoBrowser::PlatformMaximize()
{
  window_->Maximize();
}


gfx::Size
ExoBrowser::PlatformSize()
{
  return window_->GetWindowBoundsInScreen().size();
}

gfx::Point
ExoBrowser::PlatformPosition()
{
  return window_->GetWindowBoundsInScreen().origin();
}

void 
ExoBrowser::OnWidgetActivationChanged(
    views::Widget* widget, 
    bool active) 
{
  if(widget != window_.get())
    return;

  if(active) {
    /* TODO(spoluy): Notify */
  }
  else {
    /* TODO(spoluy): Notify */
  }
}


void 
ExoBrowser::DeleteDelegate() {
  Close();
}

views::View* 
ExoBrowser::GetInitiallyFocusedView() 
{
  return inspectable_web_contents()->GetView()->GetWebView();
}

bool 
ExoBrowser::CanResize() const 
{
  return true;
}

bool 
ExoBrowser::CanMaximize() const 
{
  return true;
}

base::string16 
ExoBrowser::GetWindowTitle() const 
{
  return base::UTF8ToUTF16(title_);
}

bool 
ExoBrowser::ShouldHandleSystemCommands() const 
{
  return true;
}

gfx::ImageSkia 
ExoBrowser::GetWindowAppIcon() 
{
  return *(icon_.ToImageSkia());
}

gfx::ImageSkia 
ExoBrowser::GetWindowIcon() 
{
  return GetWindowAppIcon();
}

views::Widget* 
ExoBrowser::GetWidget() 
{
  return window_.get();
}

const views::Widget* 
ExoBrowser::GetWidget() const 
{
  return window_.get();
}

views::View* 
ExoBrowser::GetContentsView() 
{
  return this;
}

bool 
ExoBrowser::ShouldDescendIntoChildForEventHandling(
    gfx::NativeView child,
    const gfx::Point& location) 
{
  /*
  // App window should claim mouse events that fall within the draggable region.
  if (draggable_region_ &&
      draggable_region_->contains(location.x(), location.y()))
    return false;

  // And the events on border for dragging resizable frameless window.
  if (!has_frame_ && CanResize()) {
    FramelessView* frame = static_cast<FramelessView*>(
        window_->non_client_view()->frame_view());
    return frame->ResizingBorderHitTest(location) == HTNOWHERE;
  }
  */
  return true;
}

views::ClientView* 
ExoBrowser::CreateClientView(
    views::Widget* widget) 
{
  return new ExoBrowserClientView(widget, this);
}

views::NonClientFrameView* 
ExoBrowser::CreateNonClientFrameView(
    views::Widget* widget) 
{
#if defined(OS_WIN)
  WinFrameView* frame_view =  new WinFrameView;
  frame_view->Init(this, widget);
  return frame_view;
#elif defined(OS_LINUX)
  if(has_frame_) {
    return new views::NativeFrameView(widget);
  } 
  else {
    FramelessView* frame_view =  new FramelessView();
    frame_view->Init(this, widget);
    return frame_view;
  }
#else
  return NULL;
#endif
}

} // namespace exo_browser
