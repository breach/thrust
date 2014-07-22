#ifndef EXO_BROWSER_COMMON_LIBRARY_MAIN_H_
#define EXO_BROWSER_COMMON_LIBRARY_MAIN_H_

#include "build/build_config.h"

extern "C" {

#if defined(OS_MACOSX)
__attribute__ ((visibility ("default")))
int ExoBrowserMain(int argc, const char* argv[]);
#endif

}

#endif // EXO_BROWSER_COMMON_LIBRARY_MAIN_H_
