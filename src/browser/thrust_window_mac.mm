// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/thrust_window.h"

#include <algorithm>

#include "base/logging.h"
#import "base/mac/scoped_nsobject.h"
#include "base/strings/string_piece.h"
#include "base/strings/sys_string_conversions.h"
#include "url/gurl.h"
#import "ui/base/cocoa/underlay_opengl_hosting_window.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "vendor/brightray/browser/inspectable_web_contents.h"
#include "vendor/brightray/browser/inspectable_web_contents_view.h"

using namespace content;


// ## ThrustWindowDelegate
//
//  Listens for event that the window should close.
@interface ThrustWindowDelegate : NSObject<NSWindowDelegate> {
 @private
  thrust_shell::ThrustWindow* window_;
}
- (id)initWithShell:(thrust_shell::ThrustWindow*) window;
@end


@implementation ThrustWindowDelegate

- (id)initWithShell:(thrust_shell::ThrustWindow*) window {
  if ((self = [super init])) {
    window_ = window;
  }
  return self;
}

// ### windowShouldClose
//
// Called when the window is about to close. If this is user generated then
// we trigger the browser kill.                                           
- (BOOL)windowShouldClose:(id)window {
  /* TODO(spolu): Check there is absolutely no leak here. Esp. in the case */
  /* the windowShouldClose handler is due to a programmatic Kill().        */
  if(!window_->is_closed()) {
    window_->Close();
    [self release];
  }
  return YES;
}

@end

@interface ThrustWindowCrWindow : UnderlayOpenGLHostingWindow {
 @private
  thrust_shell::ThrustWindow* window_;
}
- (void) setWindow:(thrust_shell::ThrustWindow*) window;
@end

@implementation ThrustWindowCrWindow

- (void) setWindow:(thrust_shell::ThrustWindow*) window {
  window_ = window;
}

@end


namespace {

NSString* kWindowTitle = @"ThrustShell";

}  // namespace

namespace thrust_shell {

void 
ThrustWindow::PlatformCleanUp() 
{
}

void 
ThrustWindow::PlatformCreateWindow(
  const gfx::Size& size)
{
  LOG(INFO) << "Create Window: " << size.width() << "x" << size.height();

  /* icon_path is ignore on OSX */
  NSRect initial_window_bounds =
      NSMakeRect(0, 0, size.width(), size.height());
  NSRect content_rect = initial_window_bounds;
  NSUInteger style_mask = NSTitledWindowMask |
                          NSClosableWindowMask |
                          NSMiniaturizableWindowMask |
                          NSResizableWindowMask;
  ThrustWindowCrWindow* window =
      [[ThrustWindowCrWindow alloc] initWithContentRect:content_rect
                                            styleMask:style_mask
                                              backing:NSBackingStoreBuffered
                                                defer:NO];
  window_ = window;
  [window setWindow:this];
  [window_ setTitle:kWindowTitle];

  /* Set the Browser window to participate in Lion Fullscreen mode. Set */
  /* Setting this flag has no effect on Snow Leopard or earlier.        */
  NSUInteger collectionBehavior = [window_ collectionBehavior];
  collectionBehavior |= NSWindowCollectionBehaviorFullScreenPrimary;
  [window_ setCollectionBehavior:collectionBehavior];

  /* Rely on the window delegate to clean us up rather than immediately     */
  /* releasing when the window gets closed. We use the delegate to do       */
  /* everything from the autorelease pool so the Browser isn't on the stack */
  /* during cleanup (ie, a window close from javascript).                   */
  [window_ setReleasedWhenClosed:NO];

  /* Create a window delegate to watch for when it's asked to go away. It */
  /* will clean itself up so we don't need to hold a reference.           */
  ThrustWindowDelegate* delegate =
      [[ThrustWindowDelegate alloc] initWithShell:this];
  [window_ setDelegate:delegate];

  NSView* view = inspectable_web_contents()->GetView()->GetNativeView();
  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

  [view setFrame:[[window_ contentView] bounds]];
  [[window_ contentView] addSubview:view];
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
