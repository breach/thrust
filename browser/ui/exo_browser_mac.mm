// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "breach/browser/ui/exo_browser.h"

#include <algorithm>

#include "base/logging.h"
#import "base/mac/scoped_nsobject.h"
#include "base/strings/string_piece.h"
#include "base/strings/sys_string_conversions.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "breach/app/resource.h"
#import "ui/base/cocoa/underlay_opengl_hosting_window.h"
#include "url/gurl.h"
#include "breach/browser/ui/exo_frame.h"

using namespace content;


// Receives notification that the window is closing so that it can start the
// tear-down process. Is responsible for deleting itself when done.
@interface ExoBrowserWindowDelegate : NSObject<NSWindowDelegate> {
 @private
  breach::ExoBrowser* browser_;
}
- (id)initWithExoBrowser:(breach::ExoBrowser*)browser;
@end


@implementation ExoBrowserWindowDelegate

- (id)initWithExoBrowser:(breach::ExoBrowser*)browser {
  if ((self = [super init])) {
    browser_ = browser;
  }
  return self;
}

/* TODO(spolu): update comment */
// Called when the window is about to close. Perform the self-destruction
// sequence by getting rid of the browser and removing it and the window from
// the various global lists. By returning YES, we allow the window to be
// removed from the screen.
- (BOOL)windowShouldClose:(id)window {
  [window autorelease];
  /* TODO(spolu): Adapt to ExoBrowser */
  //browser_.is_closed_ = true;
  [self release];

  return YES;
}

@end

@interface ExoBrowserCrWindow : UnderlayOpenGLHostingWindow {
 @private
  breach::ExoBrowser* browser_;
}
- (void) setExoBrowser:(breach::ExoBrowser*) browser;
@end

@implementation ExoBrowserCrWindow

- (void) setExoBrowser:(breach::ExoBrowser*) browser {
  browser_ = browser;
}

@end


namespace {

NSString* kWindowTitle = @"Breach";

}  // namespace

namespace breach {

void 
ExoBrowser::PlatformInitialize(
    const gfx::Size& default_window_size) 
{
}

void 
ExoBrowser::PlatformCleanUp() 
{
}

void 
ExoBrowser::PlatformCreateWindow(
    int width,
    int height) 
{
  NSRect initial_window_bounds =
      NSMakeRect(0, 0, width, height);
  NSRect content_rect = initial_window_bounds;
  NSUInteger style_mask = NSTitledWindowMask |
                          NSClosableWindowMask |
                          NSMiniaturizableWindowMask |
                          NSResizableWindowMask;
  ExoBrowserCrWindow* window =
      [[ExoBrowserCrWindow alloc] initWithContentRect:content_rect
                                            styleMask:style_mask
                                              backing:NSBackingStoreBuffered
                                                defer:NO];
  window_ = window;
  [window setExoBrowser:this];
  [window_ setTitle:kWindowTitle];
  //NSView* content = [window_ contentView];

  // Set the Browser window to participate in Lion Fullscreen mode. Set
  // Setting this flag has no effect on Snow Leopard or earlier.
  NSUInteger collectionBehavior = [window_ collectionBehavior];
  collectionBehavior |= NSWindowCollectionBehaviorFullScreenPrimary;
  [window_ setCollectionBehavior:collectionBehavior];

  // Rely on the window delegate to clean us up rather than immediately
  // releasing when the window gets closed. We use the delegate to do
  // everything from the autorelease pool so the Browser isn't on the stack
  // during cleanup (ie, a window close from javascript).
  [window_ setReleasedWhenClosed:NO];

  // Create a window delegate to watch for when it's asked to go away. It will
  // clean itself up so we don't need to hold a reference.
  ExoBrowserWindowDelegate* delegate =
      [[ExoBrowserWindowDelegate alloc] initWithExoBrowser:this];
  [window_ setDelegate:delegate];

  // show the window
  [window_ makeKeyAndOrderFront:nil];
}

/*
void 
ExoBrowser::PlatformSetContents() 
{
  NSView* web_view = web_contents_->GetView()->GetNativeView();

  NSView* content = [window_ contentView];
  [content addSubview:web_view];

  NSRect frame = [content bounds];
  frame.size.height -= kURLBarHeight;
  [web_view setFrame:frame];
  [web_view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  [web_view setNeedsDisplay:YES];
}
*/

void 
ExoBrowser::PlatformSetTitle(
   const std::string& title) 
{
  NSString* title_string = base::SysUTF8ToNSString(title);
  [window_ setTitle:title_string];
}

void 
ExoBrowser::PlatformKill() 
{
  [window_ performClose:nil];
}


void 
ExoBrowser::PlatformAddPage(
    ExoFrame *frame)
{
  /* Nothing to Do? */
}


void 
ExoBrowser::PlatformRemovePage(
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  if(visible_page_ == content_view->GetNativeView()) {
    /* TODO(spolu): implement */
    visible_page_ = NULL;
  }
}


void 
ExoBrowser::PlatformShowPage(
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  if(visible_page_ != content_view->GetNativeView()) {
    if(visible_page_ != NULL) {
      /* TODO(spolu): implement */
    }
    visible_page_ = content_view->GetNativeView();
    /* TODO(spolu): implement */
  }
}



void 
ExoBrowser::PlatformSetControl(
    CONTROL_TYPE type, 
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  LOG(INFO) << "PlatformSetControl: " << type << " " << frame->name();

  switch(type) {
    case LEFT_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case RIGHT_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case TOP_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case BOTTOM_CONTROL: 
      /* TODO(spolu): implement */
      break;
    default:
      /* Nothing to do */
      ;
  }
}


void 
ExoBrowser::PlatformSetControlDimension(
    CONTROL_TYPE type, 
    int size)
{
  LOG(INFO) << "PlatformSetControlDimension: " 
            << type << " " << size;

  switch(type) {
    case LEFT_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case RIGHT_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case TOP_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case BOTTOM_CONTROL: 
      /* TODO(spolu): implement */
      break;
    default:
      /* Nothing to do */
      ;
  }
}


void 
ExoBrowser::PlatformUnsetControl(
    CONTROL_TYPE type, 
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  LOG(INFO) << "PlatformUnsetControl: " << type << " " << frame->name();

  switch(type) {
    case LEFT_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case RIGHT_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case TOP_CONTROL: 
      /* TODO(spolu): implement */
      break;
    case BOTTOM_CONTROL: 
      /* TODO(spolu): implement */
      break;
    default:
      /* Nothing to do */
      ;
  }
}



gfx::Size
ExoBrowser::PlatformSize()
{
  int w,h;
  /* TODO(spolu) read size */
  return gfx::Size(w, h);
}

gfx::Point
ExoBrowser::PlatformPosition()
{
  int x,y;
  /* TODO(spolu) read position */
  return gfx::Point(x, y);
}


}  // namespace breach
