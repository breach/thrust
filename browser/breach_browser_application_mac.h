// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_BREACH_APPLICATION_MAC_H_
#define BREACH_BROWSER_BREACH_APPLICATION_MAC_H_

#include "base/mac/scoped_sending_event.h"
#include "base/message_loop/message_pump_mac.h"

@interface BrowserCrApplication : NSApplication<CrAppProtocol,
                                                CrAppControlProtocol> {
 @private
  BOOL handlingSendEvent_;
}

// CrAppProtocol:
- (BOOL)isHandlingSendEvent;

// CrAppControlProtocol:
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;

- (IBAction)newDocument:(id)sender;

@end

#endif  // BREACH_BROWSER_BREACH_APPLICATION_MAC_H_
