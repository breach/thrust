// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/thrust_window.h"

#include <algorithm>

#include "base/logging.h"
#import  "base/mac/scoped_nsobject.h"
#include "base/strings/string_piece.h"
#include "base/strings/sys_string_conversions.h"
#include "url/gurl.h"
#import  "ui/base/cocoa/underlay_opengl_hosting_window.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "vendor/brightray/browser/inspectable_web_contents.h"
#include "vendor/brightray/browser/inspectable_web_contents_view.h"

#include "src/api/thrust_window_binding.h"
#import  "src/browser/ui/cocoa/event_processing_window.h"

using namespace content;

static const CGFloat kThrustWindowCornerRadius = 4.0;

@interface NSView (PrivateMethods)
- (CGFloat)roundedCornerRadius;
@end

// ## ThrustNSWindowDelegate
//
//  Listens for event that the window should close.
@interface ThrustNSWindowDelegate : NSObject<NSWindowDelegate> {
 @private
  thrust_shell::ThrustWindow* window_;
  BOOL acceptsFirstMouse_;
}
- (id)initWithWindow:(thrust_shell::ThrustWindow*)window;
- (void)setAcceptsFirstMouse:(BOOL)accept;
@end

@implementation ThrustNSWindowDelegate

- (id)initWithWindow:(thrust_shell::ThrustWindow*)window {
  if((self = [super init])) {
    window_ = window;
    acceptsFirstMouse_ = NO;
  }
  return self;
}

- (void)setAcceptsFirstMouse:(BOOL)accept {
  acceptsFirstMouse_ = accept;
}

- (void)windowDidBecomeMain:(NSNotification*)notification {
  content::WebContents* web_contents = window_->GetWebContents();
  if(!web_contents) {
    return;
  }

  web_contents->RestoreFocus();

  content::RenderWidgetHostView* rwhv = web_contents->GetRenderWidgetHostView();
  if(rwhv) {
    rwhv->SetActive(true);
  }

  window_->GetBinding()->EmitFocus();
}

- (void)windowDidResignMain:(NSNotification*)notification {
  content::WebContents* web_contents = window_->GetWebContents();
  if(!web_contents) {
    return;
  }

  web_contents->StoreFocus();

  content::RenderWidgetHostView* rwhv = web_contents->GetRenderWidgetHostView();
  if(rwhv) {
    rwhv->SetActive(false);
  }

  window_->GetBinding()->EmitBlur();
}

- (void)windowDidResize:(NSNotification*)notification {
  if(!window_->HasFrame()) {
    window_->ClipWebView();
  }
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
  if (!window_->HasFrame()) {
    NSWindow* window = window_->GetNativeWindow();
    [[window standardWindowButton:NSWindowFullScreenButton] setHidden:YES];
  }
}

- (void)windowWillClose:(NSNotification*)notification {
  window_->GetBinding()->EmitClosed();
  [self autorelease];
}

- (BOOL)acceptsFirstMouse:(NSEvent*)event {
  return acceptsFirstMouse_;
}

// ### windowShouldClose
//
// Called when the window is about to close. If this is user generated then
// we trigger the browser kill.                                           
- (BOOL)windowShouldClose:(id)window {
  // When user tries to close the window by clicking the close button, we do
  // not close the window immediately, instead we try to close the web page
  // fisrt, and when the web page is closed the window will also be closed.
  window_->Close();
  return NO;
}

@end

@interface ThrustNSWindow : EventProcessingWindow {
 @private
  thrust_shell::ThrustWindow* window_;
  bool                        enable_larger_than_screen_;
}
- (void)setWindow:(thrust_shell::ThrustWindow*)window;
- (void)setEnableLargerThanScreen:(bool)enable;
@end

@implementation ThrustNSWindow

- (void)setWindow:(thrust_shell::ThrustWindow*)window {
  window_ = window;
}

- (void)setEnableLargerThanScreen:(bool)enable {
  enable_larger_than_screen_ = enable;
}

// Enable the window to be larger than screen.
- (NSRect)constrainFrameRect:(NSRect)frameRect toScreen:(NSScreen*)screen {
  if (enable_larger_than_screen_)
    return frameRect;
  else
    return [super constrainFrameRect:frameRect toScreen:screen];
}

- (IBAction)reload:(id)sender {
  content::WebContents* web_contents = window_->GetWebContents();
  content::NavigationController::LoadURLParams params(web_contents->GetURL());
  web_contents->GetController().LoadURLWithParams(params);
}

/*
- (IBAction)showDevTools:(id)sender {
  shell_->OpenDevTools();
}
*/

@end

@interface ControlRegionView : NSView {
 @private
  thrust_shell::ThrustWindow* window_; // Weak; owns self.
}
@end

@implementation ControlRegionView

- (id)initWithWindow:(thrust_shell::ThrustWindow*)window {
  if ((self = [super init]))
    window_ = window;
  return self;
}

- (BOOL)mouseDownCanMoveWindow {
  return NO;
}

- (NSView*)hitTest:(NSPoint)point {
  SkRegion* draggable_region = window_->GetDraggableRegion();
  NSView* webView = window_->GetWebContents()->GetNativeView();
  NSInteger webViewHeight = NSHeight([webView bounds]);
  if(draggable_region && 
     draggable_region->contains(point.x, webViewHeight - point.y)) {
    return nil;
  }
  return self;
}

- (void)mouseDown:(NSEvent*)event {
  /* TODO(spolu) */
}

- (void)mouseDragged:(NSEvent*)event {
  /* TODO(spolu) */
}

@end


namespace {

NSString* kWindowTitle = @"ThrustShell";

}  // namespace

namespace thrust_shell {

void 
ThrustWindow::PlatformCleanUp() 
{
  [window_ release];
}

void 
ThrustWindow::PlatformCreateWindow(
  const gfx::Size& size)
{
  LOG(INFO) << "Create Window: " << size.width() << "x" << size.height();
  int width = size.width();
  int height = size.height();

  NSRect main_screen_rect = [[[NSScreen screens] objectAtIndex:0] frame];
  NSRect cocoa_bounds = NSMakeRect(
      round((NSWidth(main_screen_rect) - width) / 2) ,
      round((NSHeight(main_screen_rect) - height) / 2),
      width,
      height);

  ThrustNSWindow* window = [[ThrustNSWindow alloc]
      initWithContentRect:cocoa_bounds
                styleMask:NSTitledWindowMask | NSClosableWindowMask |
                          NSMiniaturizableWindowMask | NSResizableWindowMask |
                          NSTexturedBackgroundWindowMask
                  backing:NSBackingStoreBuffered
                    defer:YES];

  [window setWindow:this];
  window_ = window;

  /* We will manage window's lifetime, in PlatformCleanup */
  [window_ setReleasedWhenClosed:NO];

  /* Create a window delegate to watch for when it's asked to go away. It */
  /* will clean itself up so we don't need to hold a reference.           */
  ThrustNSWindowDelegate* delegate =
      [[ThrustNSWindowDelegate alloc] initWithWindow:this];
  [window_ setDelegate:delegate];

  [window_ setTitle:kWindowTitle];

  // On OS X the initial window size doesn't include window frame.
  bool use_content_size = false;
  /* TODO(spolu): Option to add */
  //options.Get(switches::kUseContentSize, &use_content_size);
  if(has_frame_ && !use_content_size) {
    Resize(width, height);
  }

  // Enable the NSView to accept first mouse event.
  bool acceptsFirstMouse = false;
  /* TODO(spolu): Option to add */
  //options.Get(switches::kAcceptFirstMouse, &acceptsFirstMouse);
  [delegate setAcceptsFirstMouse:acceptsFirstMouse];

  /* Set the Browser window to participate in Lion Fullscreen mode. Set */
  /* Setting this flag has no effect on Snow Leopard or earlier.        */
  NSUInteger collectionBehavior = [window_ collectionBehavior];
  collectionBehavior |= NSWindowCollectionBehaviorFullScreenPrimary;
  [window_ setCollectionBehavior:collectionBehavior];

  NSView* view = inspectable_web_contents()->GetView()->GetNativeView();
  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

  InstallView();
}

void 
ThrustWindow::InstallView() 
{
  NSView* view = inspectable_web_contents()->GetView()->GetNativeView();
  if (has_frame_) {
    // Add layer with white background for the contents view.
    base::scoped_nsobject<CALayer> layer([[CALayer alloc] init]);
    [layer setBackgroundColor:CGColorGetConstantColor(kCGColorWhite)];
    [view setLayer:layer];
    [view setFrame:[[window_ contentView] bounds]];
    [[window_ contentView] addSubview:view];
  } 
  else {
    NSView* frameView = [[window_ contentView] superview];
    [view setFrame:[frameView bounds]];
    [frameView addSubview:view];

    ClipWebView();

    [[window_ standardWindowButton:NSWindowZoomButton] setHidden:YES];
    [[window_ standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
    [[window_ standardWindowButton:NSWindowCloseButton] setHidden:YES];
    [[window_ standardWindowButton:NSWindowFullScreenButton] setHidden:YES];
  }
}

void 
ThrustWindow::UninstallView() 
{
  NSView* view = inspectable_web_contents()->GetView()->GetNativeView();
  [view removeFromSuperview];
}

void 
ThrustWindow::ClipWebView() 
{
  NSView* view = GetWebContents()->GetNativeView();
  view.layer.masksToBounds = YES;
  view.layer.cornerRadius = kThrustWindowCornerRadius;
}

void 
ThrustWindow::PlatformShow() 
{
  [window_ makeKeyAndOrderFront:nil];
}

void 
ThrustWindow::PlatformClose() 
{
  [window_ performClose:nil];
}

void 
ThrustWindow::PlatformCloseImmediately() 
{
  [window_ close];
}


void 
ThrustWindow::PlatformSetTitle(
  const std::string& title) 
{
  NSString* title_string = base::SysUTF8ToNSString(title);
  [window_ setTitle:title_string];
}



void
ThrustWindow::PlatformFocus(
  bool focus)
{
  if(focus) {
    [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    [window_ makeKeyAndOrderFront:nil];
  }
  else {
    [window_ orderBack:nil];
  }
}

void
ThrustWindow::PlatformMaximize()
{
  [window_ zoom:nil];
}

void
ThrustWindow::PlatformUnMaximize()
{
  [window_ zoom:nil];
}

void
ThrustWindow::PlatformMinimize()
{
  [window_ miniaturize:nil];
}

void
ThrustWindow::PlatformRestore()
{
  [window_ deminiaturize:nil];
}


gfx::Size
ThrustWindow::PlatformSize()
{
  NSRect frame = [window_ frame];
  return gfx::Size(frame.size.width, frame.size.height);
}

gfx::Point
ThrustWindow::PlatformPosition()
{
  NSRect frame = [window_ frame];
  NSScreen* screen = [[NSScreen screens] objectAtIndex:0];

  return gfx::Point(frame.origin.x,
      NSHeight([screen frame]) - frame.origin.y - frame.size.height);
}

void
ThrustWindow::PlatformMove(
  int x, 
  int y) 
{
  NSScreen* screen = [[NSScreen screens] objectAtIndex:0];
  [window_ setFrameTopLeftPoint:NSMakePoint(x, NSHeight([screen frame]) - y)];
}

void
ThrustWindow::PlatformResize(
  int width, 
  int height) 
{
  NSRect frame = [window_ frame];
  frame.origin.y += frame.size.height; // remove the old height
  frame.origin.y -= height; // add the new height
  frame.size.width = width;
  frame.size.height = height;
  [window_ setFrame:frame
            display:YES];
}

gfx::NativeWindow
ThrustWindow::PlatformGetNativeWindow() 
{
  return window_;
}

gfx::Size 
ThrustWindow::PlatformContentSize() 
{
  NSRect bounds = [[window_ contentView] bounds];
  return gfx::Size(bounds.size.width, bounds.size.height);
}

bool
ThrustWindow::PlatformIsMaximized()
{
  return [window_ isZoomed];
}

bool
ThrustWindow::PlatformIsMinimized()
{
  return [window_ isMiniaturized];
}


void 
ThrustWindow::PlatformSetContentSize(
    int width, int height)
{
  NSRect frame_nsrect = [window_ frame];
  NSSize frame = frame_nsrect.size;
  NSSize content = [window_ contentRectForFrameRect:frame_nsrect].size;

  width = width + frame.width - content.width;
  height = height + frame.height - content.height;
  frame_nsrect.origin.y -= height - frame_nsrect.size.height;
  frame_nsrect.size.width = width;
  frame_nsrect.size.height = height;
  [window_ setFrame:frame_nsrect display:YES];
}

void 
ThrustWindow::PlatformSetMenu(
    ui::MenuModel* menu_model) 
{
  /* No action on MacOSX should use ThrustMenu::SetApplicationMenu. */
}

} // namespace thrust_shell
