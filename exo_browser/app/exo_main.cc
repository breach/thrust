// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "content/public/app/content_main.h"
#include "exo/exo_browser/app/main_delegate.h"
#include "sandbox/win/src/sandbox_types.h"

#if defined(OS_WIN)
#include "content/public/app/startup_helper_win.h"
#endif

#if defined(OS_MACOSX)
#include "main_mac.h"
#endif

#if defined(OS_WIN)

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  sandbox::SandboxInterfaceInfo sandbox_info = {0};
  content::InitializeSandboxInfo(&sandbox_info);
  exo_browser::ExoBrowserMainDelegate delegate;
  return content::ContentMain(instance, &sandbox_info, &delegate);
}

#else

int main(int argc, const char** argv) {
#if defined(OS_MACOSX)
  // Do the delegate work in main_mac to avoid having to export the
  // delegate types. (from the Content Shell)
  return ::ContentMain(argc, argv);
#else
  exo_browser::ExoBrowserMainDelegate delegate;
  return content::ContentMain(argc, argv, &delegate);
#endif  // OS_MACOSX
}

#endif  // OS_POSIX
