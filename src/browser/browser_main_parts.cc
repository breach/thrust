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
#include "src/browser/exo_shell.h"
#include "src/browser/session/exo_session.h"
#include "src/net/net_log.h"
#include "src/api/api.h"
#include "src/api/api_server.h"
#include "src/api/exo_shell_binding.h"
#include "src/api/exo_session_binding.h"


using namespace content;

namespace exo_shell {

// static
ExoShellMainParts* ExoShellMainParts::self_ = NULL;


ExoShellMainParts::ExoShellMainParts()
  : system_session_(NULL)
{
  DCHECK(!self_) << "Cannot have two ExoShellBrowserMainParts";
  self_ = this;
  api_ = new API();
}

ExoShellMainParts::~ExoShellMainParts() {
}

// static
ExoShellMainParts* 
ExoShellMainParts::Get() 
{
  DCHECK(self_);
  return self_;
}

brightray::BrowserContext* 
ExoShellMainParts::CreateBrowserContext() {
  if(system_session_ == NULL) {
    /* We create an off the record session to be used internally. */
    /* This session has a dummy cookie store. Stores nothing.     */
    system_session_ = new ExoSession(true, "system_session", true);
  }
  return system_session_;
}

void 
ExoShellMainParts::PreMainMessageLoopRun() 
{
  brightray::BrowserMainParts::PreMainMessageLoopRun();
  net_log_.reset(new ExoShellNetLog());

  /* TODO(spolu): Get Path form command line */
  //CommandLine* command_line = CommandLine::ForCurrentProcess();
  //
  api_->InstallBinding("shell", new ExoShellBindingFactory());
  api_->InstallBinding("session", new ExoSessionBindingFactory());
  
  base::FilePath path;
  base::GetTempDir(&path);
  api_server_.reset(new APIServer(api_, path.Append("_exo_shell.sock")));
  api_server_->Start();

  /*
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE, base::Bind(&ExoShellMainParts::Startup));
  */
}

void ExoShellMainParts::PostMainMessageLoopRun() 
{
  brightray::BrowserMainParts::PostMainMessageLoopRun();
  /* system_session_ is cleaned up in the above call. */

  /* TODO(spolu): Cleanup Remaining ExoSessions? */
}

}  // namespace
