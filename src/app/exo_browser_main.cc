// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "content/public/app/content_main.h"
#include "v8/include/v8.h"
#include "third_party/node/src/node.h"
#include "exo_browser/src/app/main_delegate.h"
#include <iostream>

#if defined(OS_WIN)
#include "sandbox/win/src/sandbox_types.h"
#include "content/public/app/startup_helper_win.h"
#endif

#if defined(OS_MACOSX)
#include "third_party/node/src/force_modules.h"
#include "main_mac.h"
#endif

#if defined(OS_WIN)

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  sandbox::SandboxInterfaceInfo sandbox_info = {0};
  content::InitializeSandboxInfo(&sandbox_info);
  exo_browser::ExoBrowserMainDelegate delegate;

  content::ContentMainParams params(&delegate);
  params.instance = instance;
  params.sandbox_info = &sandbox_info;
  return content::ContentMain(params);
}

#else

int main(int argc, const char** argv) {
  for(int i = 0; i < argc; i ++) {
    if(strcmp(argv[i], "--no-chrome") == 0) {
      std::cout << "OPTION: --no-chrome" << std::endl;
      return node::Start(argc, (char **)argv);
    }
  }

#if defined(OS_MACOSX)
  // Do the delegate work in main_mac to avoid having to export the
  // delegate types. (from the Content Shell)
  return ::ContentMain(argc, argv);
#else
  exo_browser::ExoBrowserMainDelegate delegate;
  content::ContentMainParams params(&delegate);
  params.argc = argc;
  params.argv = argv;
  return content::ContentMain(params);
#endif  // OS_MACOSX
}

#endif  // OS_POSIX
