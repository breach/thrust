// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_DEVTOOLS_DELEGATE_H_
#define EXO_BROWSER_DEVTOOLS_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/devtools_http_handler_delegate.h"

namespace content{
class BrowserContext;
class DevToolsHttpHandler;
}


namespace exo_browser {

class ExoBrowserDevToolsDelegate : public content::DevToolsHttpHandlerDelegate {
 public:
  explicit ExoBrowserDevToolsDelegate();
  virtual ~ExoBrowserDevToolsDelegate();

  // Stops http server.
  void Stop();

  // DevToolsHttpProtocolHandler::Delegate overrides.
  virtual std::string GetDiscoveryPageHTML() OVERRIDE;
  virtual bool BundlesFrontendResources() OVERRIDE;
  virtual base::FilePath GetDebugFrontendDir() OVERRIDE;
  virtual std::string GetPageThumbnailData(const GURL& url) OVERRIDE;
  virtual content::RenderViewHost* CreateNewTarget() OVERRIDE;
  virtual TargetType GetTargetType(content::RenderViewHost*) OVERRIDE;
  virtual std::string GetViewDescription(content::RenderViewHost*) OVERRIDE;
  virtual scoped_refptr<net::StreamListenSocket> CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) OVERRIDE;

  content::DevToolsHttpHandler* devtools_http_handler() {
    return devtools_http_handler_;
  }

 private:
  content::DevToolsHttpHandler* devtools_http_handler_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserDevToolsDelegate);
};

} // namespace exo_browser

#endif // EXO_BROWSER_DEVTOOLS_DELEGATE_H_
