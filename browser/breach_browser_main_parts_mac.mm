// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "breach/browser/breach_browser_main_parts.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/bundle_locations.h"
#include "base/mac/scoped_nsobject.h"
#include "breach/browser/breach_application_mac.h"

namespace breach {

void 
BreachBrowserMainParts::PreMainMessageLoopStart() 
{
  // Force the NSApplication subclass to be used.
  [BreachCrApplication sharedApplication];

  /* TODO(spolu): removed from content (not in chrome) */
  /*
  base::scoped_nsobject<NSNib> nib(
      [[NSNib alloc] initWithNibNamed:@"MainMenu"
                               bundle:base::mac::FrameworkBundle()]);
  [nib instantiateNibWithOwner:NSApp topLevelObjects:nil];
  */
}

}  // namespace content
