// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_MAC_THRUST_SHELL_APPLICATION_MAC_H_
#define THRUST_SHELL_BROWSER_MAC_THRUST_SHELL_APPLICATION_MAC_H_

#include "base/mac/scoped_sending_event.h"
#include "base/message_loop/message_pump_mac.h"

@interface ThrustShellApplication : NSApplication<CrAppProtocol,
                                               CrAppControlProtocol> {
 @private
  BOOL handlingSendEvent_;
}

+ (ThrustShellApplication*)sharedApplication;

// CrAppProtocol:
- (BOOL)isHandlingSendEvent;

// CrAppControlProtocol:
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;

@end

#endif // THRUST_SHELL_BROWSER_MAC_THRUST_SHELL_APPLICATION_MAC_H_
