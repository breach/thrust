// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_BROWSER_MAC_EXO_SHELL_APPLICATION_MAC_H_
#define EXO_SHELL_BROWSER_MAC_EXO_SHELL_APPLICATION_MAC_H_

#include "base/mac/scoped_sending_event.h"
#include "base/message_loop/message_pump_mac.h"

@interface ExoShellApplication : NSApplication<CrAppProtocol,
                                               CrAppControlProtocol> {
 @private
  BOOL handlingSendEvent_;
}

+ (ExoShellApplication*)sharedApplication;

// CrAppProtocol:
- (BOOL)isHandlingSendEvent;

// CrAppControlProtocol:
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;

@end

#endif // EXO_SHELL_BROWSER_MAC_EXO_SHELL_APPLICATION_MAC_H_
