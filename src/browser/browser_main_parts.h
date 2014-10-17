// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_BROWSER_MAIN_PARTS_H_
#define THRUST_SHELL_BROWSER_BROWSER_MAIN_PARTS_H_

#include "brightray/browser/browser_main_parts.h"

#include "base/basictypes.h"
#include "base/threading/thread.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

namespace net {
class NetLog;
}

namespace content {
struct MainFunctionParams;
}

namespace thrust_shell {

class ThrustSession;
class API;
class APIServer;

class ThrustShellMainParts : public brightray::BrowserMainParts {
 public:

  explicit ThrustShellMainParts();
  virtual ~ThrustShellMainParts();

  static ThrustShellMainParts* Get();

  /* TODO(spolu) Add App object similar to atom-shell Browser. */

  // Implementations of brightray::BrowserMainParts.
  virtual brightray::BrowserContext* CreateBrowserContext() OVERRIDE;

  // BrowserMainParts overrides.
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;
#if defined(OS_MACOSX) 
  virtual void PreMainMessageLoopStart() OVERRIDE; 
  virtual void PostDestroyThreads() OVERRIDE;
#endif

  net::NetLog* net_log() { 
    return net_log_.get(); 
  }

 private:
  scoped_ptr<net::NetLog> net_log_;

  static ThrustShellMainParts*          self_;
  ThrustSession*                        system_session_;

  API*                                  api_;
  scoped_ptr<APIServer>                 api_server_;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellMainParts);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_BROWSER_MAIN_PARTS_H_
