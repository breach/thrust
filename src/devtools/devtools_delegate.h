// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_DEVTOOLS_DELEGATE_H_
#define THRUST_SHELL_DEVTOOLS_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/devtools_http_handler_delegate.h"

namespace content{
class BrowserContext;
class DevToolsHttpHandler;
}

namespace thrust_shell {

class ThrustSession;

class ThrustShellDevToolsDelegate : public content::DevToolsHttpHandlerDelegate {
 public:
  explicit ThrustShellDevToolsDelegate(ThrustSession* session);

  // Stop (and destroy this)
  void Stop();

  // DevToolsHttpProtocolHandler::Delegate overrides.
  virtual std::string GetDiscoveryPageHTML() OVERRIDE;
  virtual bool BundlesFrontendResources() OVERRIDE;
  virtual base::FilePath GetDebugFrontendDir() OVERRIDE;
  virtual std::string GetPageThumbnailData(const GURL& url) OVERRIDE;

  virtual scoped_ptr<content::DevToolsTarget> 
    CreateNewTarget(const GURL& url) OVERRIDE;
  virtual void EnumerateTargets(TargetCallback callback) OVERRIDE;

  virtual scoped_ptr<net::StreamListenSocket> CreateSocketForTethering(         
      net::StreamListenSocket::Delegate* delegate,                              
      std::string* name) OVERRIDE;             

  content::DevToolsHttpHandler* devtools_http_handler() {
    return devtools_http_handler_;
  }

 private:
  virtual ~ThrustShellDevToolsDelegate();

  content::DevToolsHttpHandler*    devtools_http_handler_;
  ThrustSession*                   session_;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellDevToolsDelegate);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_DEVTOOLS_DELEGATE_H_
