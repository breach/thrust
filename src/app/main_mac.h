// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_APP_MAIN_MAC_H_
#define EXO_BROWSER_APP_MAIN_MAC_H_

#include "base/basictypes.h"

#if defined(OS_MACOSX)
extern "C" {
__attribute__((visibility("default")))
int ContentMain(int argc,
                const char** argv);
}  // extern "C"
#endif // OS_MACOSX

#endif // EXO_BROWSER_APP_MAIN_MAC_H_
