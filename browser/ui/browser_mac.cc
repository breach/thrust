// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/ui/browser.h"

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

using namespace content;

#if !defined(MAC_OS_X_VERSION_10_7) || \
    MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7

enum {
  NSWindowCollectionBehaviorFullScreenPrimary = 1 << 7,
  NSWindowCollectionBehaviorFullScreenAuxiliary = 1 << 8
};

#endif // MAC_OS_X_VERSION_10_7

// Receives notification that the window is closing so that it can start the
// tear-down process. Is responsible for deleting itself when done.
@interface BrowserWindowDelegate : NSObject<NSWindowDelegate> {
 @private
  breach::Browser* browser_;
}
- (id)initWithBrowser:(breach::Browser*)browser;
@end

@implementation BrowserWindowDelegate

- (id)initWithBrowser:(breach::Browser*)browser {
  if ((self = [super init])) {
    browser_ = browser;
  }
  return self;
}

// Called when the window is about to close. Perform the self-destruction
// sequence by getting rid of the browser and removing it and the window from
// the various global lists. By returning YES, we allow the window to be
// removed from the screen.
- (BOOL)windowShouldClose:(id)window {
  [window autorelease];
  delete browser_;
  [self release];

  return YES;
}

- (void)performAction:(id)sender {
  browser_->ActionPerformed([sender tag]);
}

- (void)takeURLStringValueFrom:(id)sender {
  browser_->URLEntered(base::SysNSStringToUTF8([sender stringValue]));
}

@end

@interface BrowserCrWindow : UnderlayOpenGLHostingWindow {
 @private
  breach::Browser* browser_;
}
- (void)setBrowser:(breach::Browser*)browser;
- (void)showDevTools:(id)sender;
@end

@implementation BrowserCrWindow

- (void)setBrowser:(breach::Browser*)browser {
  browser_ = browser;
}

- (void)showDevTools:(id)sender {
  browser_->ShowDevTools();
}

@end

namespace {

NSString* kWindowTitle = @"Breach";

// Layout constants (in view coordinates)
const CGFloat kButtonWidth = 72;
const CGFloat kURLBarHeight = 24;

// The minimum size of the window's content (in view coordinates)
const CGFloat kMinimumWindowWidth = 400;
const CGFloat kMinimumWindowHeight = 300;

void 
MakeBrowserButton(
    NSRect* rect,
    NSString* title,
    NSView* parent,
    int control,
    NSView* target,
    NSString* key,
    NSUInteger modifier) 
{
  base::scoped_nsobject<NSButton> button(
      [[NSButton alloc] initWithFrame:*rect]);
  [button setTitle:title];
  [button setBezelStyle:NSSmallSquareBezelStyle];
  [button setAutoresizingMask:(NSViewMaxXMargin | NSViewMinYMargin)];
  [button setTarget:target];
  [button setAction:@selector(performAction:)];
  [button setTag:control];
  [button setKeyEquivalent:key];
  [button setKeyEquivalentModifierMask:modifier];
  [parent addSubview:button];
  rect->origin.x += kButtonWidth;
}

}  // namespace

namespace breach {

void 
Browser::PlatformInitialize(
    const gfx::Size& default_window_size) 
{
}

void 
Browser::PlatformCleanUp() 
{
}

void 
Browser::PlatformEnableUIControl(
    UIControl control, 
    bool is_enabled) 
{
  if (headless_)
    return;

  int id;
  switch (control) {
    case BACK_BUTTON:
      id = IDC_NAV_BACK;
      break;
    case FORWARD_BUTTON:
      id = IDC_NAV_FORWARD;
      break;
    case STOP_BUTTON:
      id = IDC_NAV_STOP;
      break;
    default:
      NOTREACHED() << "Unknown UI control";
      return;
  }
  [[[window_ contentView] viewWithTag:id] setEnabled:is_enabled];
}

void 
Browser::PlatformSetAddressBarURL(
    const GURL& url) 
{
  if (headless_)
    return;

  NSString* url_string = base::SysUTF8ToNSString(url.spec());
  [url_edit_view_ setStringValue:url_string];
}

void 
Browser::PlatformSetIsLoading(
    bool loading) 
{
}

void 
Browser::PlatformCreateWindow(
    int width,
    int height) 
{
  if (headless_)
    return;

  NSRect initial_window_bounds =
      NSMakeRect(0, 0, width, height + kURLBarHeight);
  NSRect content_rect = initial_window_bounds;
  NSUInteger style_mask = NSTitledWindowMask |
                          NSClosableWindowMask |
                          NSMiniaturizableWindowMask |
                          NSResizableWindowMask;
  BrowserCrWindow* window =
      [[BrowserCrWindow alloc] initWithContentRect:content_rect
                                       styleMask:style_mask
                                         backing:NSBackingStoreBuffered
                                           defer:NO];
  window_ = window;
  [window setBrowser:this];
  [window_ setTitle:kWindowTitle];
  NSView* content = [window_ contentView];

  // If the window is allowed to get too small, it will wreck the view bindings.
  NSSize min_size = NSMakeSize(kMinimumWindowWidth, kMinimumWindowHeight);
  min_size = [content convertSize:min_size toView:nil];
  // Note that this takes window coordinates.
  [window_ setContentMinSize:min_size];

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
  BrowserWindowDelegate* delegate =
      [[BrowserWindowDelegate alloc] initWithBrowser:this];
  [window_ setDelegate:delegate];

  NSRect button_frame =
      NSMakeRect(0, NSMaxY(initial_window_bounds) - kURLBarHeight,
                 kButtonWidth, kURLBarHeight);

  MakeBrowserButton(&button_frame, @"Back", content, IDC_NAV_BACK,
                  (NSView*)delegate, @"[", NSCommandKeyMask);
  MakeBrowserButton(&button_frame, @"Forward", content, IDC_NAV_FORWARD,
                  (NSView*)delegate, @"]", NSCommandKeyMask);
  MakeBrowserButton(&button_frame, @"Reload", content, IDC_NAV_RELOAD,
                  (NSView*)delegate, @"r", NSCommandKeyMask);
  MakeBrowserButton(&button_frame, @"Stop", content, IDC_NAV_STOP,
                  (NSView*)delegate, @".", NSCommandKeyMask);

  button_frame.size.width =
      NSWidth(initial_window_bounds) - NSMinX(button_frame);
  base::scoped_nsobject<NSTextField> url_edit_view(
      [[NSTextField alloc] initWithFrame:button_frame]);
  [content addSubview:url_edit_view];
  [url_edit_view setAutoresizingMask:(NSViewWidthSizable | NSViewMinYMargin)];
  [url_edit_view setTarget:delegate];
  [url_edit_view setAction:@selector(takeURLStringValueFrom:)];
  [[url_edit_view cell] setWraps:NO];
  [[url_edit_view cell] setScrollable:YES];
  url_edit_view_ = url_edit_view.get();

  // show the window
  [window_ makeKeyAndOrderFront:nil];
}

void 
Browser::PlatformSetContents() 
{
  NSView* web_view = web_contents_->GetView()->GetNativeView();

  if (headless_) {
    NSRect frame = NSMakeRect(
        0, 0, kDefaultTestWindowWidthDip, kDefaultTestWindowHeightDip);
    [web_view setFrame:frame];
    [web_view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
    return;
  }

  NSView* content = [window_ contentView];
  [content addSubview:web_view];

  NSRect frame = [content bounds];
  frame.size.height -= kURLBarHeight;
  [web_view setFrame:frame];
  [web_view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  [web_view setNeedsDisplay:YES];
}

void 
Browser::PlatformResizeSubViews() 
{
  // Not needed; subviews are bound.
}

void 
Browser::PlatformSetTitle(const string16& title) 
{
  if (headless_)
    return;

  NSString* title_string = base::SysUTF16ToNSString(title);
  [window_ setTitle:title_string];
}

void 
Browser::Close() 
{
  if (headless_)
    delete this;
  else
    [window_ performClose:nil];
}

void 
Browser::ActionPerformed(int control) 
{
  switch (control) {
    case IDC_NAV_BACK:
      GoBackOrForward(-1);
      break;
    case IDC_NAV_FORWARD:
      GoBackOrForward(1);
      break;
    case IDC_NAV_RELOAD:
      Reload();
      break;
    case IDC_NAV_STOP:
      Stop();
      break;
  }
}

void 
Browser::URLEntered(std::string url_string) 
{
  if (!url_string.empty()) {
    GURL url(url_string);
    if (!url.has_scheme())
      url = GURL("http://" + url_string);
    LoadURL(url);
  }
}

void 
Browser::HandleKeyboardEvent(
    WebContents* source,
    const NativeWebKeyboardEvent& event) 
{
  if (event.skip_in_browser)
    return;

  // The event handling to get this strictly right is a tangle; cheat here a bit
  // by just letting the menus have a chance at it.
  if ([event.os_event type] == NSKeyDown) {
    if (([event.os_event modifierFlags] & NSCommandKeyMask) &&
        [[event.os_event characters] isEqual:@"l"]) {
      [window_ makeFirstResponder:url_edit_view_];
      return;
    }

    [[NSApp mainMenu] performKeyEquivalent:event.os_event];
  }
}

}  // namespace breach
