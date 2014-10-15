// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#include "src/browser/browser_main_parts.h"

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "cc/base/switches.h"
#include "net/base/net_module.h"
#include "net/base/net_util.h"
#include "url/gurl.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/main_function_params.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/browser_thread.h"

#include "src/common/switches.h"
#include "src/browser/thrust_window.h"
#include "src/browser/session/thrust_session.h"
#include "src/net/net_log.h"
#include "src/api/api.h"
#include "src/api/api_server.h"
#include "src/api/thrust_window_binding.h"
#include "src/api/thrust_session_binding.h"
#include "src/api/thrust_menu_binding.h"


using namespace content;

namespace thrust_shell {

// static
ThrustShellMainParts* ThrustShellMainParts::self_ = NULL;


ThrustShellMainParts::ThrustShellMainParts()
  : system_session_(NULL)
{
  DCHECK(!self_) << "Cannot have two ThrustShellBrowserMainParts";
  self_ = this;
  api_ = new API();
}

ThrustShellMainParts::~ThrustShellMainParts() {
}

// static
ThrustShellMainParts* 
ThrustShellMainParts::Get() 
{
  DCHECK(self_);
  return self_;
}

brightray::BrowserContext* 
ThrustShellMainParts::CreateBrowserContext() {
  if(system_session_ == NULL) {
    /* We create an off the record session to be used internally. */
    /* This session has a dummy cookie store. Stores nothing.     */
    system_session_ = new ThrustSession(NULL, true, "system_session", true);
  }
  return system_session_;
}

void 
ThrustShellMainParts::PreMainMessageLoopRun() 
{
  brightray::BrowserMainParts::PreMainMessageLoopRun();
  net_log_.reset(new ThrustShellNetLog());

  CommandLine* command_line = CommandLine::ForCurrentProcess();
  base::FilePath path = 
    command_line->GetSwitchValuePath(switches::kThrustShellSocketPath);

  CHECK(!path.empty()) << "Command line switch `" 
                       << switches::kThrustShellSocketPath
                       << "` must be specified. Aborting.";

  api_->InstallBinding("window", new ThrustWindowBindingFactory());
  api_->InstallBinding("session", new ThrustSessionBindingFactory());
  api_->InstallBinding("menu", new ThrustMenuBindingFactory());

  api_server_.reset(new APIServer(api_, path));
  api_server_->Start();
}

void ThrustShellMainParts::PostMainMessageLoopRun() 
{
  brightray::BrowserMainParts::PostMainMessageLoopRun();
  /* system_session_ is cleaned up in the above call. */

  /* TODO(spolu): Cleanup Remaining ThrustSessions? */
}

}  // namespace
