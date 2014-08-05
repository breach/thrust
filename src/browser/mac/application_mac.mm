// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/mac/application_mac.h"

#include "base/auto_reset.h"
#include "url/gurl.h"

#include "src/browser/exo_shell.h"
#include "src/browser/browser_client.h"

#include  <sys/types.h>
#include  <signal.h>

@implementation ExoShellApplication

+ (ExoShellApplication*)sharedApplication {
  return (ExoShellApplication*)[super sharedApplication];
}

- (BOOL)isHandlingSendEvent {
  return handlingSendEvent_;
}

- (void)sendEvent:(NSEvent*)event {
  base::AutoReset<BOOL> scoper(&handlingSendEvent_, YES);
  [super sendEvent:event];
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
  handlingSendEvent_ = handlingSendEvent;
}

- (void)terminate:(id)sender {
  /* We override the normal termination and simply send a kill signal to the */
  /* process to comply with the linux behaviour.                             */
  /* TODO(spolu): Send event to API */
  kill(getpid(), SIGINT);
}

@end
