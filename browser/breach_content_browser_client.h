// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_BREACH_CONTENT_BROWSER_CLIENT_H_
#define BREACH_BROWSER_BREACH_CONTENT_BROWSER_CLIENT_H_

#include <string>
#include <map>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/web_contents_view.h"

namespace breach {

/* TODO(spolu): renaming post file creation */
class ShellBrowserContext;
class ShellBrowserMainParts;
class ShellResourceDispatcherHostDelegate;
class RenderProcessHost;

class BreachContentBrowserClient : public content::ContentBrowserClient {
 public:
  // Gets the current instance.
  static BreachContentBrowserClient* Get();

  BreachContentBrowserClient();
  virtual ~BreachContentBrowserClient();

  // ContentBrowserClient overrides.
  virtual BrowserMainParts* CreateBrowserMainParts(
      const content::MainFunctionParams& parameters) OVERRIDE;

  virtual void AppendExtraCommandLineSwitches(
      CommandLine* command_line,
      int child_process_id) OVERRIDE;

  /* TODO(spolu): check removal */
  //virtual void OverrideWebkitPrefs(content::RenderViewHost* render_view_host,
  //                               const GURL& url,
  //                               WebPreferences* prefs) OVERRIDE;

  virtual std::string GetApplicationLocale() OVERRIDE;

  virtual void ResourceDispatcherHostCreated() OVERRIDE;

  virtual AccessTokenStore* CreateAccessTokenStore() OVERRIDE;

  virtual std::string GetDefaultDownloadName() OVERRIDE;

  virtual void BrowserURLHandlerCreated(
      content::BrowserURLHandler* handler) OVERRIDE;

  virtual bool ShouldTryToUseExistingProcessHost(
      BrowserContext* content::browser_context, 
      const GURL& url) OVERRIDE;

  virtual bool IsSuitableHost(
      content::RenderProcessHost* process_host,
      const GURL& site_url) OVERRIDE;

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


  /* TODO(spolu): renaming post file creation */
  ShellBrowserContext* browser_context();
  ShellBrowserContext* off_the_record_browser_context();

  /* TODO(spolu): renaming post file creation */
  ShellResourceDispatcherHostDelegate* resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
  /* TODO(spolu): renaming post file creation */
  ShellBrowserMainParts* browser_main_parts() {
    return browser_main_parts_;
  }

 private:
  /* TODO(spolu): renaming post file creation */
  ShellBrowserContext* ShellBrowserContextForBrowserContext(
      BrowserContext* content_browser_context);

  /* TODO(spolu): renaming post file creation */
  scoped_ptr<ShellResourceDispatcherHostDelegate>
      resource_dispatcher_host_delegate_;

  /* TODO(spolu): renaming post file creation */
  ShellBrowserMainParts* browser_main_parts_;
};

}  // namespace breach

#endif  // BREACH_BROWSER_BREACH_CONTENT_BROWSER_CLIENT_H_
