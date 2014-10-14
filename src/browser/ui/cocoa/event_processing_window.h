// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2014 GitHub, Inc.
// See the LICENSE file.

#ifndef EXO_SHELL_BROWSER_UI_COCOA_EVENT_PROCESSING_WINDOW_H_
#define EXO_SHELL_BROWSER_UI_COCOA_EVENT_PROCESSING_WINDOW_H_

#import <Cocoa/Cocoa.h>

// Override NSWindow to access unhandled keyboard events (for command
// processing); subclassing NSWindow is the only method to do
// this.
@interface EventProcessingWindow : NSWindow {
 @private
  BOOL redispatchingEvent_;
  BOOL eventHandled_;
}

// Sends a key event to |NSApp sendEvent:|, but also makes sure that it's not
// short-circuited to the RWHV. This is used to send keyboard events to the menu
// and the cmd-` handler if a keyboard event comes back unhandled from the
// renderer. The event must be of type |NSKeyDown|, |NSKeyUp|, or
// |NSFlagsChanged|.
// Returns |YES| if |event| has been handled.
- (BOOL)redispatchKeyEvent:(NSEvent*)event;

- (BOOL)performKeyEquivalent:(NSEvent*)theEvent;
@end

#endif  // EXO_SHELL_BROWSER_UI_COCOA_EVENT_PROCESSING_WINDOW_H_
