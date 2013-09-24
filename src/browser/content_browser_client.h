// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_CONTENT_BROWSER_CLIENT_H_
#define EXO_BROWSER_BROWSER_CONTENT_BROWSER_CLIENT_H_

#include <string>
#include <map>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/web_contents_view.h"

namespace exo_browser {

class ExoBrowserMainParts;
class ExoBrowserContext;
class ExoBrowserResourceDispatcherHostDelegate;
class RenderProcessHost;

class ExoBrowserContentBrowserClient : public content::ContentBrowserClient {
 public:
  // Gets the current instance.
  static ExoBrowserContentBrowserClient* Get();

  ExoBrowserContentBrowserClient();
  virtual ~ExoBrowserContentBrowserClient();

  // ContentBrowserClient Override.
  virtual content::BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) OVERRIDE;

  virtual void AppendExtraCommandLineSwitches(
      CommandLine* command_line,
      int child_process_id) OVERRIDE;

  virtual void OverrideWebkitPrefs(content::RenderViewHost* render_view_host,
                                   const GURL& url,
                                   WebPreferences* prefs) OVERRIDE;

  virtual void ResourceDispatcherHostCreated() OVERRIDE;

  /* TODO(spolu): Reintroduce AccessTokenStore */
  //virtual content::AccessTokenStore* CreateAccessTokenStore() OVERRIDE;

  virtual std::string GetDefaultDownloadName() OVERRIDE;

  virtual content::WebContentsViewDelegate* GetWebContentsViewDelegate(
      content::WebContents* web_contents) OVERRIDE;

  virtual void BrowserURLHandlerCreated(
      content::BrowserURLHandler* handler) OVERRIDE;

  virtual void RenderProcessHostCreated(
      content::RenderProcessHost* host) OVERRIDE;

  virtual net::URLRequestContextGetter* CreateRequestContext(
      content::BrowserContext* browser_context,
      content::ProtocolHandlerMap* protocol_handlers) OVERRIDE;

  virtual net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      content::BrowserContext* browser_context,
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers) OVERRIDE;

  virtual bool IsHandledURL(const GURL& url) OVERRIDE;

  ExoBrowserResourceDispatcherHostDelegate* 
  resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
  ExoBrowserMainParts* browser_main_parts() {
    return browser_main_parts_;
  }

 private:
  ExoBrowserContext* ExoBrowserContextForBrowserContext(
      content::BrowserContext* content_browser_context);

  scoped_ptr<ExoBrowserResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  ExoBrowserMainParts* browser_main_parts_;
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_CONTENT_BROWSER_CLIENT_H_
