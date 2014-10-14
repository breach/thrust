// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "src/browser/browser_main_parts.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/bundle_locations.h"
#include "base/mac/scoped_nsobject.h"

#include "src/browser/mac/application_mac.h"

namespace thrust_shell {

void 
ThrustShellMainParts::PreMainMessageLoopStart() 
{
  // Force the NSApplication subclass to be used.
  [ThrustShellApplication sharedApplication];

  // Prevent Cocoa from turning command-line arguments into
  // |-application:openFiles:|, since we already handle them directly.
  [[NSUserDefaults standardUserDefaults]
      setObject:@"NO" forKey:@"NSTreatUnknownArgumentsAsOpen"];
}

void 
ThrustShellMainParts::PostDestroyThreads() {
  [[ThrustShellApplication sharedApplication] setDelegate:nil];
}

} // namespace thrust_shell
