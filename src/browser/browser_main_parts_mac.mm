// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "src/browser/browser_main_parts.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/bundle_locations.h"
#include "base/mac/scoped_nsobject.h"

#include "src/browser/mac/browser_application_mac.h"

namespace exo_browser {

void 
ExoBrowserMainParts::PreMainMessageLoopStart() 
{
  // Force the NSApplication subclass to be used.
  NSApplication* application = [ExoBrowserApplication sharedApplication];

  // Prevent Cocoa from turning command-line arguments into
  // |-application:openFiles:|, since we already handle them directly.
  [[NSUserDefaults standardUserDefaults]
      setObject:@"NO" forKey:@"NSTreatUnknownArgumentsAsOpen"];
}

void 
ExoBrowserMainParts::PostDestroyThreads() {
  [[ExoBrowserApplication sharedApplication] setDelegate:nil];
}

} // namespace exo_browser
