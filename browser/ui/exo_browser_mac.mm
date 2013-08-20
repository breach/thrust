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
  visible_page_ = NULL;
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
  NSView* content = [window_ contentView];

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

  NSRect rect = [window_ frame];

  control_left_box_ = [[[NSView alloc] initWithFrame:rect] autorelease];
  control_right_box_ = [[[NSView alloc] initWithFrame:rect] autorelease];
  control_top_box_ = [[[NSView alloc] initWithFrame:rect] autorelease];
  control_bottom_box_ = [[[NSView alloc] initWithFrame:rect] autorelease];
  vertical_box_ = [[[NSView alloc] initWithFrame:rect] autorelease];
  pages_box_ = [[[NSView alloc] initWithFrame:rect] autorelease];

  [content addSubview: control_left_box_];
  [content addSubview: vertical_box_];
  [content addSubview: control_right_box_];
  [vertical_box_ addSubview: control_top_box_];
  [vertical_box_ addSubview: pages_box_];
  [vertical_box_ addSubview: control_bottom_box_];
  
  [control_left_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [control_right_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [control_top_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [control_bottom_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [vertical_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];

  NSDictionary *content_dict = 
    NSDictionaryOfVariableBindings(control_left_box_,
                                   control_right_box_,
                                   vertical_box_);
  [content addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"H:|-0-[control_left_box_(>=200)]-[vertical_box_(>=300)]-[control_right_box_(>=200)]-0-|"
                          options:0
                          metrics:nil
                            views:content_dict]];


  NSDictionary *vertical_dict = 
    NSDictionaryOfVariableBindings(control_top_box_,
                                   control_bottom_box_,
                                   pages_box_);

  [vertical_box_ addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:|-0-[control_top_box_(>=100)]-[pages_box_(>=400)]-[control_bottom_box_(>=100)]-0-|"
                          options:0
                          metrics:nil
                            views:vertical_dict]];
   
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
    [visible_page_ removeFromSuperview];
    visible_page_ = NULL;
  }
}


void 
ExoBrowser::PlatformShowPage(
    ExoFrame *frame)
{
  NSView* web_view = frame->web_contents_->GetView()->GetNativeView();
  [pages_box_ setSubviews:[NSArray array]];
  [pages_box_ addSubview: web_view];

  NSRect rect = [[window_ contentView] bounds];
  [web_view setFrame:rect];
  [web_view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  [web_view setNeedsDisplay:YES];
/*
  WebContentsView* content_view = frame->web_contents_->GetView();
  if(visible_page_ != content_view->GetNativeView()) {
    if(visible_page_ != NULL) {
      [visible_page_ removeFromSuperview];
    }
    visible_page_ = content_view->GetNativeView();
    [pages_box_ addSubview: visible_page_];
  }
*/
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
      [control_left_box_ addSubview: content_view->GetNativeView()];
      /* TODO(spolu): implement */
      break;
    case RIGHT_CONTROL: 
      [control_right_box_ addSubview: content_view->GetNativeView()];
      /* TODO(spolu): implement */
      break;
    case TOP_CONTROL: 
      [control_top_box_ addSubview: content_view->GetNativeView()];
      /* TODO(spolu): implement */
      break;
    case BOTTOM_CONTROL: 
      [control_bottom_box_ addSubview: content_view->GetNativeView()];
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
  int w = 0, h = 0;
  /* TODO(spolu) read size */
  return gfx::Size(w, h);
}

gfx::Point
ExoBrowser::PlatformPosition()
{
  int x = 0, y = 0;
  /* TODO(spolu) read position */
  return gfx::Point(x, y);
}


}  // namespace breach
