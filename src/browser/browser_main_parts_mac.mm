// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "exo_browser/src/browser/browser_main_parts.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/bundle_locations.h"
#include "base/mac/scoped_nsobject.h"
#include "exo_browser/src/browser/browser_application_mac.h"

namespace exo_browser {

void 
ExoBrowserMainParts::PreMainMessageLoopStart() 
{
  // Force the NSApplication subclass to be used.
  [BrowserCrApplication sharedApplication];
}

} // namespace exo_browser
