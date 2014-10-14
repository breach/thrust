// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/app/library_main.h"

#include "content/public/app/content_main.h"
#include "src/app/main_delegate.h"

int ThrustShellMain(int argc, const char* argv[]) {
  thrust_shell::MainDelegate delegate;
  content::ContentMainParams params(&delegate);
  params.argc = argc;
  params.argv = argv;
  return content::ContentMain(params);
}
