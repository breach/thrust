// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/exo_shell.h"

#include <algorithm>

#include "base/logging.h"
#import "base/mac/scoped_nsobject.h"
#include "base/strings/string_piece.h"
#include "base/strings/sys_string_conversions.h"
#include "url/gurl.h"
#import "ui/base/cocoa/underlay_opengl_hosting_window.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "vendor/brightray/browser/inspectable_web_contents_view.h"

using namespace content;


// ## ExoShellWindowDelegagte
//
//  Listens for event that the window should close.
@interface ExoShellWindowDelegate : NSObject<NSWindowDelegate> {
 @private
  exo_shell::ExoShell* shell_;
}
- (id)initWithShell:(exo_shell::ExoShell*) shell;
@end


@implementation ExoShellWindowDelegate

- (id)initWithShell:(exo_shell::ExoShell*) shell {
  if ((self = [super init])) {
    shell_ = shell;
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
  if(!shell_->is_closed()) {
    shell_->Close();
    [self release];
  }
  return YES;
}

@end

@interface ExoShellCrWindow : UnderlayOpenGLHostingWindow {
 @private
  exo_shell::ExoShell* shell_;
}
- (void) setShell:(exo_shell::ExoShell*) shell;
@end

@implementation ExoShellCrWindow

- (void) setShell:(exo_shell::ExoShell*) shell {
  shell_ = shell;
}

@end


namespace {

NSString* kWindowTitle = @"ExoShell";

}  // namespace

namespace exo_shell {

void 
ExoShell::PlatformCleanUp() 
{
}

void 
ExoShell::PlatformCreateWindow(
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
  ExoShellCrWindow* window =
      [[ExoShellCrWindow alloc] initWithContentRect:content_rect
                                            styleMask:style_mask
                                              backing:NSBackingStoreBuffered
                                                defer:NO];
  window_ = window;
  [window setShell:this];
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
  ExoShellWindowDelegate* delegate =
      [[ExoShellWindowDelegate alloc] initWithShell:this];
  [window_ setDelegate:delegate];

  NSView* view = inspectable_web_contents()->GetView()->GetNativeView();
  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

  [view setFrame:[[window_ contentView] bounds]];
  [[window_ contentView] addSubview:view];

}

void 
ExoShell::PlatformShow() 
{
  [window_ makeKeyAndOrderFront:nil];
}

void 
ExoShell::PlatformClose() 
{
  [window_ performClose:nil];
}


void 
ExoShell::PlatformSetTitle(
   const std::string& title) 
{
  NSString* title_string = base::SysUTF8ToNSString(title);
  [window_ setTitle:title_string];
}



void
ExoShell::PlatformFocus(bool focus)
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
ExoShell::PlatformMaximize()
{
  [window_ zoom:nil];
}

void
ExoShell::PlatformUnMaximize()
{
  [window_ zoom:nil];
}

void
ExoShell::PlatformMinimize()
{
  [window_ miniaturize:nil];
}

void
ExoShell::PlatformRestore()
{
  [window_ deminiaturize:nil];
}


gfx::Size
ExoShell::PlatformSize()
{
  NSRect frame = [window_ frame];
  return gfx::Size(frame.size.width, frame.size.height);
}

gfx::Point
ExoShell::PlatformPosition()
{
  NSRect frame = [window_ frame];
  NSScreen* screen = [[NSScreen screens] objectAtIndex:0];

  return gfx::Point(frame.origin.x,
      NSHeight([screen frame]) - frame.origin.y - frame.size.height);
}

void
ExoShell::PlatformMove(int x, int y) {
  NSScreen* screen = [[NSScreen screens] objectAtIndex:0];
  [window_ setFrameTopLeftPoint:NSMakePoint(x, NSHeight([screen frame]) - y)];
}

void
ExoShell::PlatformResize(int width, int height) {
  NSRect frame = [window_ frame];
  frame.origin.y += frame.size.height; // remove the old height
  frame.origin.y -= height; // add the new height
  frame.size.width = width;
  frame.size.height = height;
  [window_ setFrame:frame
            display:YES];
}

} // namespace exo_shell
