// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/app/breach_main_mac.h"
#include "content/public/app/content_main.h"
#include "breach/app/breach_main_delegate.h"

#if defined(OS_MACOSX)
int ContentMain(int argc,
                const char** argv) {
  breach::BreachMainDelegate delegate;
  return content::ContentMain(argc, argv, &delegate);
}
#endif  // OS_MACOSX
