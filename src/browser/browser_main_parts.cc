// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "exo_browser/src/browser/browser_main_parts.h"

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#include "net/base/net_module.h"
#include "net/base/net_util.h"
#include "url/gurl.h"
#include "grit/net_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/browser/ui/exo_browser.h"
#include "exo_browser/src/devtools/devtools_delegate.h"
#include "exo_browser/src/net/net_log.h"
#include "exo_browser/src/node/node_thread.h"


using namespace content;

namespace exo_browser {

namespace {


base::StringPiece 
PlatformResourceProvider(
    int key) 
{
  if (key == IDR_DIR_HEADER_HTML) {
    base::StringPiece html_data =
        ui::ResourceBundle::GetSharedInstance().GetRawDataResource(
            IDR_DIR_HEADER_HTML);
    return html_data;
  }
  return base::StringPiece();
}

}  // namespace

ExoBrowserMainParts::ExoBrowserMainParts(
    const MainFunctionParams& parameters)
    : BrowserMainParts(), 
      parameters_(parameters), 
      run_message_loop_(true) 
{
}

ExoBrowserMainParts::~ExoBrowserMainParts() {
}

int
ExoBrowserMainParts::PreCreateThreads()
{
  /* We create the NodeThread which will host NodeJS */
  NodeThread::Get()->Start();
  return 0;
}

#if !defined(OS_MACOSX)
void 
ExoBrowserMainParts::PreMainMessageLoopStart()
{
}
#endif

void 
ExoBrowserMainParts::PostMainMessageLoopStart() 
{
}

void 
ExoBrowserMainParts::PreEarlyInitialization() 
{
}

void 
ExoBrowserMainParts::PreMainMessageLoopRun() 
{
  net_log_.reset(new ExoBrowserNetLog());

  ExoBrowser::Initialize();
  net::NetModule::SetResourceProvider(PlatformResourceProvider);

  devtools_delegate_.reset(new ExoBrowserDevToolsDelegate());

  if(parameters_.ui_task) {
    parameters_.ui_task->Run();
    delete parameters_.ui_task;
    run_message_loop_ = false;
  }
}

bool ExoBrowserMainParts::MainMessageLoopRun(int* result_code)  
{
  return !run_message_loop_;
}

void ExoBrowserMainParts::PostMainMessageLoopRun() 
{
  if (devtools_delegate_)
    devtools_delegate_->Stop();
  /* TODO(spolu): Cleanup Remaining ExoSession? */
}

}  // namespace
