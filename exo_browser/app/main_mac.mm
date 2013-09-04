// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "content/public/app/content_main.h"
#include "exo/exo_browser/app/exo_main_mac.h"
#include "exo/exo_browser/app/main_delegate.h"

#if defined(OS_MACOSX)
int ContentMain(int argc,
                const char** argv) {
  exo_browser::ExoBrowserMainDelegate delegate;
  return content::ContentMain(argc, argv, &delegate);
}
#endif // OS_MACOSX
