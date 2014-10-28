// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_CONTENT_BROWSER_H_
#define THRUST_SHELL_BROWSER_CONTENT_BROWSER_H_

#include <string>
#include <map>

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/content_browser_client.h"

#include "brightray/browser/browser_client.h"

namespace thrust_shell {

class ThrustShellMainParts;
class ThrustShellContext;
class ThrustShellResourceDispatcherHostDelegate;
class RenderProcessHost;
class ThrustSession;

class ThrustShellBrowserClient : public brightray::BrowserClient {
 public:

  ThrustShellBrowserClient();
  virtual ~ThrustShellBrowserClient();

  /****************************************************************************/
  /* STATIC API */
  /****************************************************************************/
  // ### Get
  //
  // Returns the ThrustShellBrowserClient singleton
  static ThrustShellBrowserClient* Get();

  // ### OverrideAppName
  //
  // Override application name.
  static void OverrideAppName(const std::string& name);

  // ### OverrideAppVersion
  //
  // Override application version.
  static void OverrideAppVersion(const std::string& name);

  // ### GetAppName
  //
  // Retrieves the Executable File Product Name or the overridden App Name
  static std::string GetAppName();

  // ### GetAppVersion
  //
  // Retrieves the Executable File Version
  static std::string GetAppVersion();


  /****************************************************************************/
  /* CONTENTBROWSERCLIENT IMPLEMENTATION */
  /****************************************************************************/
  virtual void AppendExtraCommandLineSwitches(
      base::CommandLine* command_line,
      int child_process_id) OVERRIDE;

  virtual void OverrideWebkitPrefs(content::RenderViewHost* render_view_host,
                                   const GURL& url,
                                   content::WebPreferences* prefs) OVERRIDE;

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
  // ### RegisterThrustSession
  // ```
  // @session {ThrustSession} the session to register
  // ```
  // Lets ThrustSession register themselves when constructed so that they can be
  // retrieved from their underlying BrowserContext when needed.
  // see ThrustSessionForBrowserContext
  void RegisterThrustSession(ThrustSession* session);

  // ### UnRegisterThrustSession
  // ```
  // @session {ThrustSession} the session to unregister
  // ```
  void UnRegisterThrustSession(ThrustSession* session);

  // ### ThrustSessionFroBrowserContext
  // ```
  // @content_browser_context {BrowserContext}
  // ```
  // Retrieves the ThrustSession wrapping the given content::BrowserContext
  ThrustSession* ThrustSessionForBrowserContext(
      content::BrowserContext* browser_context);

  /****************************************************************************/
  /* ACCESSORS */
  /****************************************************************************/
  ThrustShellResourceDispatcherHostDelegate* 
  resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
  ThrustSession* system_session();


 private:
  virtual brightray::BrowserMainParts* OverrideCreateBrowserMainParts(
      const content::MainFunctionParams&) OVERRIDE;

  scoped_ptr<ThrustShellResourceDispatcherHostDelegate>
                            resource_dispatcher_host_delegate_;

  std::vector<ThrustSession*>           sessions_;

  static ThrustShellBrowserClient*      self_;
  static std::string                    app_name_override_;
  static std::string                    app_version_override_;
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_CONTENT_BROWSER_CLIENT_H_
