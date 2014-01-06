// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_BROWSER_MAIN_PARTS_H_
#define EXO_BROWSER_BROWSER_BROWSER_MAIN_PARTS_H_

#include "base/basictypes.h"
#include "base/threading/thread.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

namespace base {
class Thread;
}

namespace net {
class NetLog;
}

namespace content {
struct MainFunctionParams;
}

namespace exo_browser {

class NodeThread;

class ExoBrowserMainParts : public content::BrowserMainParts {
 public:

  explicit ExoBrowserMainParts(
      const content::MainFunctionParams& parameters);
  virtual ~ExoBrowserMainParts();

  // BrowserMainParts overrides.
  virtual int PreCreateThreads() OVERRIDE;
  virtual void PreEarlyInitialization() OVERRIDE;
  virtual void PreMainMessageLoopStart() OVERRIDE;
  virtual void PostMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

  net::NetLog* net_log() { 
    return net_log_.get(); 
  }

 private:
  scoped_ptr<net::NetLog> net_log_;

  // For running content_browsertests.
  const content::MainFunctionParams& parameters_;
  bool run_message_loop_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserMainParts);
};

} // namespace exo_browser

#endif  // EXO_BROWSER_BROWSER_BROWSER_MAIN_PARTS_H_
