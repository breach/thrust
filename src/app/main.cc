// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/app/main.h"

#include "src/app/library_main.h"
#include "src/app/main_delegate.h"

#include "content/public/app/content_main.h"
#include "sandbox/win/src/sandbox_types.h"

#if defined(OS_WIN)

#include "content/public/app/startup_helper_win.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, wchar_t*, int) {
  sandbox::SandboxInterfaceInfo sandbox_info = {0};
  content::InitializeSandboxInfo(&sandbox_info);
  exo_shell::MainDelegate delegate;

  content::ContentMainParams params(&delegate);
  params.instance = instance;
  params.sandbox_info = &sandbox_info;
  return content::ContentMain(params);
}

// defined(OS_WIN)
#elif defined(OS_LINUX) 

int main(int argc, const char* argv[]) {
  exo_shell::MainDelegate delegate;
  content::ContentMainParams params(&delegate);
  params.argc = argc;
  params.argv = argv;
  return content::ContentMain(params);
}

// defined(OS_LINUX)
#else 

int main(int argc, const char* argv[]) {
  return ExoShellMain(argc, argv);
}

// defined(OS_MACOSX)
#endif
