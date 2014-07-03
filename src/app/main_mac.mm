// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "content/public/app/content_main.h"
#include "exo_browser/src/app/main_mac.h"
#include "exo_browser/src/app/main_delegate.h"

#if defined(OS_MACOSX)
int ContentMain(int argc, const char** argv) {
  exo_browser::ExoBrowserMainDelegate delegate;
  content::ContentMainParams params(&delegate);
  params.argc = argc;
  params.argv = argv;
  return content::ContentMain(params);
}
#endif // OS_MACOSX
