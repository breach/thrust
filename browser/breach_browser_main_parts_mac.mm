// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "breach/browser/shell_browser_main_parts.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/bundle_locations.h"
#include "base/mac/scoped_nsobject.h"
#include "content/shell/shell_application_mac.h"

namespace breach {

void 
BreachBrowserMainParts::PreMainMessageLoopStart() 
{
  /* TODO(spolu): renaming post file creation */
  // Force the NSApplication subclass to be used.
  [ShellCrApplication sharedApplication];

  base::scoped_nsobject<NSNib> nib(
      [[NSNib alloc] initWithNibNamed:@"MainMenu"
                               bundle:base::mac::FrameworkBundle()]);
  [nib instantiateNibWithOwner:NSApp topLevelObjects:nil];
}

}  // namespace content
