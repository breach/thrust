// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_BROWSER_CONTENT_BROWSER_H_
#define EXO_SHELL_BROWSER_CONTENT_BROWSER_H_

#include <string>
#include <map>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/content_browser_client.h"

#include "brightray/browser/browser_client.h"

namespace exo_shell {

class ExoShellMainParts;
class ExoShellContext;
class ExoShellResourceDispatcherHostDelegate;
class RenderProcessHost;
class ExoSession;

class ExoShellBrowserClient : public brightray::BrowserClient {
 public:

  ExoShellBrowserClient();
  virtual ~ExoShellBrowserClient();

  static ExoShellBrowserClient* Get();

  /****************************************************************************/
  /* CONTENTBROWSERCLIENT IMPLEMENTATION */
  /****************************************************************************/
  virtual void AppendExtraCommandLineSwitches(
      base::CommandLine* command_line,
      int child_process_id) OVERRIDE;

  virtual void OverrideWebkitPrefs(content::RenderViewHost* render_view_host,
                                   const GURL& url,
                                   WebPreferences* prefs) OVERRIDE;

  virtual void ResourceDispatcherHostCreated() OVERRIDE;

  virtual content::AccessTokenStore* CreateAccessTokenStore() OVERRIDE;

  virtual std::string GetDefaultDownloadName() OVERRIDE;
  virtual std::string GetApplicationLocale() OVERRIDE;

  virtual content::WebContentsViewDelegate* GetWebContentsViewDelegate(
      content::WebContents* web_contents) OVERRIDE;

  virtual net::URLRequestContextGetter* CreateRequestContext(
      content::BrowserContext* browser_context,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector protocol_interceptors) OVERRIDE;

  virtual net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      content::BrowserContext* browser_context,
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers,
      content::URLRequestInterceptorScopedVector protocol_interceptors) OVERRIDE;

  virtual bool IsHandledURL(const GURL& url) OVERRIDE;

  /****************************************************************************/
  /* EXOSESSION I/F */
  /****************************************************************************/
  // ### RegisterExoSession
  // ```
  // @session {ExoSession} the session to register
  // ```
  // Lets ExoSession register themselves when constructed so that they can be
  // retrieved from their underlying BrowserContext when needed.
  // see ExoSessionForBrowserContext
  void RegisterExoSession(ExoSession* session);

  // ### UnRegisterExoSession
  // ```
  // @session {ExoSession} the session to unregister
  // ```
  void UnRegisterExoSession(ExoSession* session);

  // ### ExoSessionFroBrowserContext
  // ```
  // @content_browser_context {BrowserContext}
  // ```
  // Retrieves the ExoSession wrapping the given content::BrowserContext
  ExoSession* ExoSessionForBrowserContext(
      content::BrowserContext* browser_context);

  /****************************************************************************/
  /* ACCESSORS */
  /****************************************************************************/
  ExoShellResourceDispatcherHostDelegate* 
  resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
  ExoSession* system_session();


 private:
  virtual brightray::BrowserMainParts* OverrideCreateBrowserMainParts(
      const content::MainFunctionParams&) OVERRIDE;

  scoped_ptr<ExoShellResourceDispatcherHostDelegate>
                            resource_dispatcher_host_delegate_;

  static ExoShellBrowserClient*    self_;
  std::vector<ExoSession*>           sessions_;
};

} // namespace exo_shell

#endif // EXO_SHELL_BROWSER_CONTENT_BROWSER_CLIENT_H_
