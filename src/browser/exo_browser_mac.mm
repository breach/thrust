// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "exo_browser/src/browser/exo_browser.h"

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
#include "exo_browser/src/app/resource.h"
#include "exo_browser/src/browser/exo_frame.h"

using namespace content;


// ## ExoBrowserWindowDelegagte
//
//  Listens for event that the window should close.
@interface ExoBrowserWindowDelegate : NSObject<NSWindowDelegate> {
 @private
  exo_browser::ExoBrowser* browser_;
}
- (id)initWithExoBrowser:(exo_browser::ExoBrowser*)browser;
@end


@implementation ExoBrowserWindowDelegate

- (id)initWithExoBrowser:(exo_browser::ExoBrowser*)browser {
  if ((self = [super init])) {
    browser_ = browser;
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
  if(!browser_->is_killed()) {
    browser_->Kill();
    [self release];
  }
  return YES;
}

@end

@interface ExoBrowserCrWindow : UnderlayOpenGLHostingWindow {
 @private
  exo_browser::ExoBrowser* browser_;
}
- (void) setExoBrowser:(exo_browser::ExoBrowser*) browser;
@end

@implementation ExoBrowserCrWindow

- (void) setExoBrowser:(exo_browser::ExoBrowser*) browser {
  browser_ = browser;
}

@end


namespace {

NSString* kWindowTitle = @"ExoBrowser";

}  // namespace

namespace exo_browser {

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
    int height,
    const bool kiosk,
    const std::string& icon_path)
{
  /* icon_path is ignore on OSX */
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
  ExoBrowserWindowDelegate* delegate =
      [[ExoBrowserWindowDelegate alloc] initWithExoBrowser:this];
  [window_ setDelegate:delegate];

  control_left_box_ = [[[NSView alloc] init] autorelease];
  control_right_box_ = [[[NSView alloc] init] autorelease];
  control_top_box_ = [[[NSView alloc] init] autorelease];
  control_bottom_box_ = [[[NSView alloc] init] autorelease];
  horizontal_box_ = [[[NSView alloc] init] autorelease];
  pages_box_ = [[[NSView alloc] init] autorelease];
  floating_box_ = NULL;

  visible_page_ = NULL;
  floating_frame_ = NULL;
  floating_box_ = NULL;

  /* Vertical Layout */
  [control_top_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [content addSubview: control_top_box_]; 
  [horizontal_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [content addSubview: horizontal_box_];
  [control_bottom_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [content addSubview: control_bottom_box_]; 

  NSDictionary *content_dict = 
    NSDictionaryOfVariableBindings(control_top_box_,
                                   control_bottom_box_,
                                   horizontal_box_);
  [content addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:|[control_top_box_][horizontal_box_][control_bottom_box_]|"
                          options:0
                          metrics:nil
                            views:content_dict]];

  NSArray * control_top_box_constraints = 
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:[control_top_box_(0)]"
                          options:0
                          metrics:nil
                            views:content_dict];
  [content addConstraints: control_top_box_constraints];
  control_top_constraint_ = [control_top_box_constraints objectAtIndex: 0];

  NSArray * control_bottom_box_constraints = 
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:[control_bottom_box_(0)]"
                          options:0
                          metrics:nil
                            views:content_dict];
  [content addConstraints: control_bottom_box_constraints];
  control_bottom_constraint_ = [control_bottom_box_constraints objectAtIndex: 0];

  [content addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:[horizontal_box_(>=100)]"
                          options:0
                          metrics:nil
                            views:content_dict]];
  [content addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"|-0-[horizontal_box_(>=100)]-0-|"
                          options:0
                          metrics:nil
                            views:content_dict]];
  [content addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"|-0-[control_top_box_]-0-|"
                          options:0
                          metrics:nil
                            views:content_dict]];
  [content addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"|-0-[control_bottom_box_]-0-|"
                          options:0
                          metrics:nil
                            views:content_dict]];


  /* Horizontal Layout */
  [control_left_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [horizontal_box_ addSubview: control_left_box_];
  [pages_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [horizontal_box_ addSubview: pages_box_];
  [control_right_box_ setTranslatesAutoresizingMaskIntoConstraints:NO];
  [horizontal_box_ addSubview: control_right_box_];
   
  NSDictionary *horizontal_dict = 
    NSDictionaryOfVariableBindings(control_left_box_,
                                   control_right_box_,
                                   pages_box_);

  [horizontal_box_ addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"|[control_left_box_][pages_box_][control_right_box_]|"
                          options:0
                          metrics:nil
                            views:horizontal_dict]];

  NSArray * control_left_box_constraints = 
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"[control_left_box_(0)]"
                          options:0
                          metrics:nil
                            views:horizontal_dict];
  [horizontal_box_ addConstraints: control_left_box_constraints];
  control_left_constraint_ = [control_left_box_constraints objectAtIndex: 0];

  NSArray * control_right_box_constraints = 
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"[control_right_box_(0)]"
                          options:0
                          metrics:nil
                            views:horizontal_dict];
  [horizontal_box_ addConstraints: control_right_box_constraints];
  control_right_constraint_ = [control_right_box_constraints objectAtIndex: 0];

  [horizontal_box_ addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"[pages_box_(>=100)]"
                          options:0
                          metrics:nil
                            views:horizontal_dict]];

  [horizontal_box_ addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:|-0-[pages_box_(>=100)]-0-|"
                          options:0
                          metrics:nil
                            views:horizontal_dict]];
  [horizontal_box_ addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:|-0-[control_left_box_]-0-|"
                          options:0
                          metrics:nil
                            views:horizontal_dict]];
  [horizontal_box_ addConstraints:
    [NSLayoutConstraint 
      constraintsWithVisualFormat:@"V:|-0-[control_right_box_]-0-|"
                          options:0
                          metrics:nil
                            views:horizontal_dict]];

  // Handle kiosk mode
  if(kiosk) {
    // TODO: these might be a bit too restrictive
    NSApplicationPresentationOptions options =
      NSApplicationPresentationHideMenuBar |
      NSApplicationPresentationHideDock |
      NSApplicationPresentationDisableHideApplication |
      NSApplicationPresentationDisableProcessSwitching |
      NSApplicationPresentationDisableAppleMenu |
      NSApplicationPresentationDisableForceQuit;

    [NSApp setPresentationOptions:options];
    [[_window contentView] enterFullScreenMode:
      [NSScreen mainScreen] withOptions:nil];
  }

  // show the window
  [window_ makeKeyAndOrderFront:nil];
}


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
  /* Nothing to Do */
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
  WebContentsView* content_view = frame->web_contents_->GetView();
  if(visible_page_ != content_view->GetNativeView()) {
    if(visible_page_ != NULL) {
      [visible_page_ removeFromSuperview];
    }
    visible_page_ = content_view->GetNativeView();

    NSRect rect = [pages_box_ bounds];
    [visible_page_ setFrame:rect];
    [visible_page_ setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
    [pages_box_ addSubview: visible_page_];

    [visible_page_ setNeedsDisplay:YES];
  }
}


void 
ExoBrowser::PlatformSetControl(
    CONTROL_TYPE type, 
    ExoFrame *frame)
{
  NSView* web_view = frame->web_contents_->GetView()->GetNativeView();
  [web_view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  LOG(INFO) << "PlatformSetControl: " << type << " " << frame->name();

  NSView* container = nil;
  switch(type) {
    case LEFT_CONTROL: 
      container = control_left_box_;
      break;
    case RIGHT_CONTROL: 
      container = control_right_box_;
      break;
    case TOP_CONTROL: 
      container = control_top_box_;
      break;
    case BOTTOM_CONTROL: 
      container = control_bottom_box_;
      break;
    default:
      /* Nothing to do */
      ;
  }
  if(container) {
    NSRect rect = [container bounds];
    [web_view setFrame:rect];
    [container addSubview: web_view];
    [web_view setNeedsDisplay:YES];
  }
}


void 
ExoBrowser::PlatformSetControlDimension(
    CONTROL_TYPE type, 
    int size)
{
  LOG(INFO) << "PlatformSetControlDimension: " 
            << type << " " << size;

  NSView* container = nil;
  NSLayoutConstraint* constraint = nil;
  switch(type) {
    case LEFT_CONTROL: 
      constraint = control_left_constraint_;
      container = control_left_box_;
      break;
    case RIGHT_CONTROL: 
      constraint = control_right_constraint_;
      container = control_right_box_;
      break;
    case TOP_CONTROL: 
      constraint = control_top_constraint_;
      container = control_top_box_;
      break;
    case BOTTOM_CONTROL: 
      constraint = control_bottom_constraint_;
      container = control_bottom_box_;
      break;
    default:
      /* Nothing to do */
      ;
  }
  if(constraint) {
    [constraint setConstant: size];
  }
}


void 
ExoBrowser::PlatformUnsetControl(
    CONTROL_TYPE type, 
    ExoFrame *frame)
{
  LOG(INFO) << "PlatformUnsetControl: " << type << " " << frame->name();

  NSView* container = nil;
  NSLayoutConstraint* constraint = nil;
  switch(type) {
    case LEFT_CONTROL: 
      container = control_left_box_;
      constraint = control_left_constraint_;
      break;
    case RIGHT_CONTROL: 
      container = control_right_box_;
      constraint = control_right_constraint_;
      break;
    case TOP_CONTROL: 
      container = control_top_box_;
      constraint = control_top_constraint_;
      break;
    case BOTTOM_CONTROL: 
      container = control_bottom_box_;
      constraint = control_bottom_constraint_;
      break;
    default:
      /* Nothing to do */
      ;
  }
  if(container) {
    [container setSubviews:[NSArray array]];
    [constraint setConstant: 0];
  }
}

void
ExoBrowser::PlatformShowFloating(
    ExoFrame* frame,
    int x, 
    int y,
    int width, 
    int height)
{
  DCHECK(floating_frame_ == NULL);
  WebContentsView* content_view = frame->web_contents_->GetView();
  floating_frame_ = content_view->GetNativeView();

  floating_box_ = [[[NSWindow alloc] initWithContentRect:NSMakeRect(x, y, width, height) 
                                               styleMask:NSBorderlessWindowMask 
                                                 backing:NSBackingStoreBuffered 
                                                   defer:NO] autorelease];
  [window_ addChildWindow:floating_box_ ordered:NSWindowAbove];

  [[floating_box_ contentView] addSubview: floating_frame_];
  [floating_frame_ setFrame: [[floating_box_ contentView] bounds]];
  [floating_frame_ setNeedsDisplay:YES];
}

void
ExoBrowser::PlatformHideFloating()
{
  DCHECK(floating_frame_ != NULL && floating_box_ != NULL);
  [window_ removeChildWindow: floating_box_];
  floating_box_ = NULL;
  [floating_frame_ removeFromSuperview];
  floating_frame_ = NULL;
}

void
ExoBrowser::PlatformFocus()
{
  [window_ makeKeyAndOrderFront:nil];
}

void
ExoBrowser::PlatformMaximize()
{
  [window_ zoom:nil];
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


} // namespace exo_browser
