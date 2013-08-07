// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_BROWSER_MAIN_PARTS_H_
#define BREACH_BROWSER_BROWSER_MAIN_PARTS_H_

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

namespace breach {

class BreachBrowserContext;
class BreachDevToolsDelegate;
class NodeThread;

class BreachBrowserMainParts : public content::BrowserMainParts {
 public:

  explicit BreachBrowserMainParts(
      const content::MainFunctionParams& parameters);
  virtual ~BreachBrowserMainParts();

  // BrowserMainParts overrides.
  virtual int PreCreateThreads() OVERRIDE;
  virtual void PreEarlyInitialization() OVERRIDE;
  virtual void PreMainMessageLoopStart() OVERRIDE;
  virtual void PostMainMessageLoopStart() OVERRIDE;
  virtual void PreMainMessageLoopRun() OVERRIDE;
  virtual bool MainMessageLoopRun(
      int* result_code) OVERRIDE;
  virtual void PostMainMessageLoopRun() OVERRIDE;

  BreachDevToolsDelegate* devtools_delegate() {
    return devtools_delegate_.get();
  }

  BreachBrowserContext* browser_context() {
    return browser_context_.get(); 
  }
  BreachBrowserContext* off_the_record_browser_context() {
    return off_the_record_browser_context_.get();
  }

  net::NetLog* net_log() { return net_log_.get(); }

 private:
  scoped_ptr<net::NetLog> net_log_;
  scoped_ptr<BreachBrowserContext> browser_context_;
  scoped_ptr<BreachBrowserContext> off_the_record_browser_context_;

  // For running content_browsertests.
  const content::MainFunctionParams& parameters_;
  bool run_message_loop_;

  scoped_ptr<BreachDevToolsDelegate> devtools_delegate_;

  DISALLOW_COPY_AND_ASSIGN(BreachBrowserMainParts);
};

}  // namespace breach

#endif  // BREACH_BROWSER_BROWSER_MAIN_PARTS_H_
