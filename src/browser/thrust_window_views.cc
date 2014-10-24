// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/thrust_window.h"

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
#include "base/strings/stringprintf.h"
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
#include "content/public/browser/native_web_keyboard_event.h"
#include "vendor/brightray/browser/inspectable_web_contents_view.h"

#include "src/browser/ui/views/menu_bar.h"
#include "src/browser/ui/views/menu_layout.h"
#include "src/browser/browser_client.h"
#include "src/api/thrust_window_binding.h"

#if defined(USE_X11)
#include "base/environment.h"
#include "base/nix/xdg_util.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/x/x11_types.h"
#include "ui/views/window/native_frame_view.h"
#include "chrome/browser/ui/libgtk2ui/unity_service.h"
#include "src/browser/ui/views/global_menu_bar_x11.h"
#include "src/browser/ui/views/frameless_view.h"
#elif defined(OS_WIN)
#include "src/browser/ui/views/win_frame_view.h"
#include "base/win/scoped_comptr.h"
#endif

using namespace content;

namespace thrust_shell {

namespace {


#if defined(USE_X11)
// Counts how many window has already been created, it will be used to set the
// window role for X11.
static int kWindowsCreated = 0;
#endif

bool IsAltKey(const content::NativeWebKeyboardEvent& event) {
#if defined(USE_X11)
  // 164 and 165 represent VK_LALT and VK_RALT.
  return event.windowsKeyCode == 164 || event.windowsKeyCode == 165;
#else
  return event.windowsKeyCode == ui::VKEY_MENU;
#endif
}

bool IsAltModifier(const content::NativeWebKeyboardEvent& event) {
  typedef content::NativeWebKeyboardEvent::Modifiers Modifiers;
  return (event.modifiers == Modifiers::AltKey) ||
         (event.modifiers == (Modifiers::AltKey | Modifiers::IsLeft)) ||
         (event.modifiers == (Modifiers::AltKey | Modifiers::IsRight));
}


class ThrustWindowClientView : public views::ClientView {
 public:
  ThrustWindowClientView(
      views::Widget* widget,
      ThrustWindow* window)
      : views::ClientView(widget, window) 
  {
  }
  virtual ~ThrustWindowClientView() {}

  virtual bool 
  CanClose() OVERRIDE 
  {
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ThrustWindowClientView);
};

}  // namespace

void 
ThrustWindow::PlatformCleanUp() 
{
  window_->RemoveObserver(this);
}

void 
ThrustWindow::PlatformCreateWindow(
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
  params.remove_standard_frame = !has_frame_;

#if defined(USE_X11)
  // Set WM_WINDOW_ROLE.
  params.wm_role_name = base::StringPrintf(
      "%s/%s/%d", "Thrust", 
      ThrustShellBrowserClient::Get()->GetAppName().c_str(),
      ++kWindowsCreated);
  // Set WM_CLASS.
  params.wm_class_name = "thrust";
  params.wm_class_class = "Thrust";
#endif

  window_->Init(params);

#if defined(USE_X11)
  // Set _GTK_THEME_VARIANT to dark if we have "dark-theme" option set.
  bool use_dark_theme = false;
  /* TODO(spolu): Add option */
  /*
  if (options.Get(switches::kDarkTheme, &use_dark_theme) && use_dark_theme) {
    XDisplay* xdisplay = gfx::GetXDisplay();
    XChangeProperty(xdisplay, GetAcceleratedWidget(),
                    XInternAtom(xdisplay, "_GTK_THEME_VARIANT", False),
                    XInternAtom(xdisplay, "UTF8_STRING", False),
                    8, PropModeReplace,
                    reinterpret_cast<const unsigned char*>("dark"),
                    4);
  }
  */

  // Before the window is mapped the SetWMSpecState can not work, so we have
  // to manually set the _NET_WM_STATE.
  bool skip_taskbar = false;
  /* TODO(spolu): Add option */
  /*
  if (options.Get(switches::kSkipTaskbar, &skip_taskbar) && skip_taskbar) {
    std::vector<::Atom> state_atom_list;
    state_atom_list.push_back(GetAtom("_NET_WM_STATE_SKIP_TASKBAR"));
    ui::SetAtomArrayProperty(GetAcceleratedWidget(), "_NET_WM_STATE", "ATOM",
                             state_atom_list);
  }
  */
#endif

  // Add web view.
  SetLayoutManager(new views::FillLayout());
  set_background(views::Background::CreateStandardPanelBackground());
  AddChildView(inspectable_web_contents()->GetView()->GetView());

  /* TODO(spolu): Add option */
  /*
  if(has_frame_ &&
     options.Get(switches::kUseContentSize, &use_content_size_) &&
     use_content_size_)
    bounds = ContentBoundsToWindowBounds(bounds);
  */

  window_->UpdateWindowIcon();
  window_->CenterWindow(bounds.size());
  Layout();
}

void 
ThrustWindow::PlatformShow() 
{
  window_->Show();
}

void 
ThrustWindow::PlatformClose() 
{
  window_->Close();
}

void 
ThrustWindow::PlatformCloseImmediately() 
{
  window_->CloseNow();
}

void 
ThrustWindow::PlatformSetTitle(
    const std::string& title) 
{
  window_->UpdateWindowTitle();
}

void 
ThrustWindow::PlatformSetFullscreen(
    bool fullscreen) 
{
  window_->SetFullscreen(fullscreen);
}

bool 
ThrustWindow::PlatformIsFullscreen() 
{
  return window_->IsFullscreen();
}

void 
ThrustWindow::PlatformSetKiosk(
    bool kiosk) 
{
  PlatformSetFullscreen(kiosk);
}

bool
ThrustWindow::PlatformIsKiosk()
{
  return PlatformIsFullscreen();
}

void
ThrustWindow::PlatformFocus(bool focus)
{
  if(focus) {
    window_->Activate();
  }
  else {
    window_->Deactivate();
  }
}

void
ThrustWindow::PlatformMaximize()
{
  window_->Maximize();
}

void
ThrustWindow::PlatformUnMaximize()
{
  window_->Restore();
}

void
ThrustWindow::PlatformMinimize()
{
  window_->Minimize();
}

void
ThrustWindow::PlatformRestore()
{
  window_->Restore();
}

gfx::Size
ThrustWindow::PlatformSize()
{
#if defined(OS_WIN)
  if(window_->IsMinimized()) {
    return window_->GetRestoredBounds().size();
  }
#endif
  return window_->GetWindowBoundsInScreen().size();
}

gfx::Size 
ThrustWindow::PlatformContentSize() 
{
  if (!has_frame_)
    return PlatformSize();

  gfx::Size content_size =
      window_->non_client_view()->frame_view()->GetBoundsForClientView().size();
  return content_size;
}

bool
ThrustWindow::PlatformIsMaximized()
{
  return window_->IsMaximized();
}

bool
ThrustWindow::PlatformIsMinimized()
{
  return window_->IsMinimized();
}

gfx::Rect
ThrustWindow::ContentBoundsToWindowBounds(
    const gfx::Rect& bounds)
{
  gfx::Rect window_bounds =
      window_->non_client_view()->GetWindowBoundsForClientBounds(bounds);
  return window_bounds;
}


void 
ThrustWindow::PlatformSetContentSize(
    int width, int height)
{
  if (!has_frame_) {
    PlatformResize(width, height);
    return;
  }

  gfx::Size size(width, height);
  gfx::Rect bounds = window_->GetWindowBoundsInScreen();
  gfx::Size new_size = 
      ContentBoundsToWindowBounds(gfx::Rect(bounds.origin(), size)).size();
  PlatformResize(size.width(), size.height());
}

gfx::Point
ThrustWindow::PlatformPosition()
{
  return window_->GetWindowBoundsInScreen().origin();
}

void
ThrustWindow::PlatformMove(int x, int y)
{
  gfx::Size size = window_->GetWindowBoundsInScreen().size();
  gfx::Rect bounds(x, y, size.width(), size.height());
  window_->SetBounds(bounds);
}

void
ThrustWindow::PlatformResize(int width, int height)
{
  gfx::Point origin = window_->GetWindowBoundsInScreen().origin();
  gfx::Rect bounds(origin.x(), origin.y(), width, height);
  window_->SetBounds(bounds);
}

gfx::NativeWindow
ThrustWindow::PlatformGetNativeWindow() 
{
  return window_->GetNativeWindow(); 
}

void 
ThrustWindow::OnWidgetActivationChanged(
    views::Widget* widget, 
    bool active) 
{
  if(widget != window_.get())
    return;

  if(active) {
    binding_->EmitFocus();
  }
  else {
    binding_->EmitBlur();
  }

  if(active && web_contents()) {
    web_contents()->Focus();
  }
}


void 
ThrustWindow::DeleteDelegate() {
  binding_->EmitClosed();
  Close();
}

views::View* 
ThrustWindow::GetInitiallyFocusedView() 
{
  return inspectable_web_contents()->GetView()->GetWebView();
}

bool 
ThrustWindow::CanResize() const 
{
  return true;
}

bool 
ThrustWindow::CanMaximize() const 
{
  return true;
}

base::string16 
ThrustWindow::GetWindowTitle() const 
{
  return base::UTF8ToUTF16(title_);
}

bool 
ThrustWindow::ShouldHandleSystemCommands() const 
{
  return true;
}

gfx::ImageSkia 
ThrustWindow::GetWindowAppIcon() 
{
  return icon_;
}

gfx::ImageSkia 
ThrustWindow::GetWindowIcon() 
{
  return GetWindowAppIcon();
}

views::Widget* 
ThrustWindow::GetWidget() 
{
  return window_.get();
}

const views::Widget* 
ThrustWindow::GetWidget() const 
{
  return window_.get();
}

views::View* 
ThrustWindow::GetContentsView() 
{
  return this;
}

void 
ThrustWindow::PlatformSetMenu(
    ui::MenuModel* menu_model) 
{
  /* TODO(spolu) Menu accelerators */
  /*
  // Clear previous accelerators.
  views::FocusManager* focus_manager = GetFocusManager();
  accelerator_table_.clear();
  focus_manager->UnregisterAccelerators(this);

  // Register accelerators with focus manager.
  accelerator_util::GenerateAcceleratorTable(&accelerator_table_, menu_model);
  accelerator_util::AcceleratorTable::const_iterator iter;
  for (iter = accelerator_table_.begin();
       iter != accelerator_table_.end();
       ++iter) {
    focus_manager->RegisterAccelerator(
        iter->first, ui::AcceleratorManager::kNormalPriority, this);
  }
  */

#if defined(USE_X11)
  if(!global_menu_bar_) {
    global_menu_bar_.reset(new GlobalMenuBarX11(this));
  }

  // Use global application menu bar when possible.
  if(global_menu_bar_ && global_menu_bar_->IsServerStarted()) {
    global_menu_bar_->SetMenu(menu_model);
    return;
  }
#endif

  /* We do not show menu relative to the window, they should be implemented */
  /* in the window main document.                                           */
  return;
}

bool 
ThrustWindow::ShouldDescendIntoChildForEventHandling(
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
ThrustWindow::CreateClientView(
    views::Widget* widget) 
{
  return new ThrustWindowClientView(widget, this);
}

views::NonClientFrameView* 
ThrustWindow::CreateNonClientFrameView(
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

} // namespace thrust_shell
