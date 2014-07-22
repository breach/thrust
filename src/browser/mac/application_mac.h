// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_MAC_EXO_BROWSER_APPLICATION_MAC_H_
#define EXO_BROWSER_BROWSER_MAC_EXO_BROWSER_APPLICATION_MAC_H_

#include "base/mac/scoped_sending_event.h"
#include "base/message_loop/message_pump_mac.h"

@interface ExoBrowserApplication : NSApplication<CrAppProtocol,
                                                 CrAppControlProtocol> {
 @private
  BOOL handlingSendEvent_;
}

+ (ExoBrowserApplication*)sharedApplication;

// CrAppProtocol:
- (BOOL)isHandlingSendEvent;

// CrAppControlProtocol:
- (void)setHandlingSendEvent:(BOOL)handlingSendEvent;

@end

#define // EXO_BROWSER_BROWSER_MAC_EXO_BROWSER_APPLICATION_MAC_H_
