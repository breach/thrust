// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/exo_shell.h"

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

namespace exo_shell {

namespace {

class ExoShellClientView : public views::ClientView {
 public:
  ExoShellClientView(
      views::Widget* widget,
      ExoShell* shell)
      : views::ClientView(widget, shell) 
  {
  }
  virtual ~ExoShellClientView() {}

  virtual bool 
  CanClose() OVERRIDE 
  {
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ExoShellClientView);
};

}  // namespace

void 
ExoShell::PlatformCleanUp() 
{
  window_->RemoveObserver(this);
}

void 
ExoShell::PlatformCreateWindow(
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
}

void 
ExoShell::PlatformShow() 
{
  window_->Show();
}

void 
ExoShell::PlatformClose() 
{
  window_->Close();
}

void 
ExoShell::PlatformSetTitle(
    const std::string& title) 
{
  window_->UpdateWindowTitle();
}

void
ExoShell::PlatformFocus(bool focus)
{
  if(focus) {
    window_->Activate();
  }
  else {
    window_->Deactivate();
  }
}

void
ExoShell::PlatformMaximize()
{
  window_->Maximize();
}

void
ExoShell::PlatformUnMaximize()
{
  window_->Restore();
}

void
ExoShell::PlatformMinimize()
{
  window_->Minimize();
}

void
ExoShell::PlatformRestore()
{
  window_->Restore();
}

gfx::Size
ExoShell::PlatformSize()
{
  return window_->GetWindowBoundsInScreen().size();
}

gfx::Point
ExoShell::PlatformPosition()
{
  return window_->GetWindowBoundsInScreen().origin();
}

void
ExoShell::PlatformMove(int x, int y)
{
	gfx::Size size = window_->GetWindowBoundsInScreen().size();

	gfx::Rect bounds(x, y, size.width(), size.height());

	window_->SetBounds(bounds);
}

void
ExoShell::PlatformResize(int width, int height)
{
	gfx::Point origin = window_->GetWindowBoundsInScreen().origin();

	gfx::Rect bounds(origin.x(), origin.y(), width, height);

	window_->SetBounds(bounds);
}

void 
ExoShell::OnWidgetActivationChanged(
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
ExoShell::DeleteDelegate() {
  Close();
}

views::View* 
ExoShell::GetInitiallyFocusedView() 
{
  return inspectable_web_contents()->GetView()->GetWebView();
}

bool 
ExoShell::CanResize() const 
{
  return true;
}

bool 
ExoShell::CanMaximize() const 
{
  return true;
}

base::string16 
ExoShell::GetWindowTitle() const 
{
  return base::UTF8ToUTF16(title_);
}

bool 
ExoShell::ShouldHandleSystemCommands() const 
{
  return true;
}

gfx::ImageSkia 
ExoShell::GetWindowAppIcon() 
{
  return *(icon_.ToImageSkia());
}

gfx::ImageSkia 
ExoShell::GetWindowIcon() 
{
  return GetWindowAppIcon();
}

views::Widget* 
ExoShell::GetWidget() 
{
  return window_.get();
}

const views::Widget* 
ExoShell::GetWidget() const 
{
  return window_.get();
}

views::View* 
ExoShell::GetContentsView() 
{
  return this;
}

bool 
ExoShell::ShouldDescendIntoChildForEventHandling(
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
ExoShell::CreateClientView(
    views::Widget* widget) 
{
  return new ExoShellClientView(widget, this);
}

views::NonClientFrameView* 
ExoShell::CreateNonClientFrameView(
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

} // namespace exo_shell
