#include "common/library_main.h"

#include "common/main_delegate.h"
#include "content/public/app/content_main.h"

int ExoBrowserMain(int argc, const char* argv[]) {
  exo_browser::MainDelegate delegate;
  content::ContentMainParams params(&delegate);
  params.argc = argc;
  params.argv = argv;
  return content::ContentMain(params);
}
