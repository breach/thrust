// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_BROWSER_MAIN_PARTS_H_
#define EXO_BROWSER_BROWSER_BROWSER_MAIN_PARTS_H_

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

namespace exo_browser {

class ExoSession;
class ApiHandler;

class ExoBrowserMainParts : public brightray::BrowserMainParts {
 public:

  explicit ExoBrowserMainParts();
  virtual ~ExoBrowserMainParts();

  static ExoBrowserMainParts* Get();

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

  static ExoBrowserMainParts*        self_;
  ExoSession*                        system_session_;
  scoped_ptr<ApiHandler>             api_handler_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserMainParts);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_BROWSER_MAIN_PARTS_H_
