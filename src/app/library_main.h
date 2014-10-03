#ifndef EXO_SHELL_APP_LIBRARY_MAIN_H_
#define EXO_SHELL_APP_LIBRARY_MAIN_H_

#include "build/build_config.h"

extern "C" {

#if defined(OS_MACOSX)
__attribute__ ((visibility ("default")))
int ExoShellMain(int argc, const char* argv[]);
#endif

}

#endif // EXO_SHELL_APP_LIBRARY_MAIN_H_
