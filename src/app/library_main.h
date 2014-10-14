// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_APP_LIBRARY_MAIN_H_
#define THRUST_SHELL_APP_LIBRARY_MAIN_H_

#include "build/build_config.h"

extern "C" {

#if defined(OS_MACOSX)
__attribute__ ((visibility ("default")))
int ThrustShellMain(int argc, const char* argv[]);
#endif

}

#endif // THRUST_SHELL_APP_LIBRARY_MAIN_H_
