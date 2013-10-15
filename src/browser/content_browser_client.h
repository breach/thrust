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
class ExoSession;

class ExoBrowserContentBrowserClient : public content::ContentBrowserClient {
 public:
  // Gets the current instance.
  static ExoBrowserContentBrowserClient* Get();

  ExoBrowserContentBrowserClient();
  virtual ~ExoBrowserContentBrowserClient();

  /****************************************************************************/
  /*                  CONTENTBROWSERCLIENT IMPLEMENTATION                     */
  /****************************************************************************/
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

  /****************************************************************************/
  /*                            EXOSESSION I/F                                */
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
  /*                              ACCESSORS                                   */
  /****************************************************************************/
  ExoBrowserResourceDispatcherHostDelegate* 
  resource_dispatcher_host_delegate() {
    return resource_dispatcher_host_delegate_.get();
  }
  ExoBrowserMainParts* browser_main_parts() {
    return browser_main_parts_;
  }


 private:
  scoped_ptr<ExoBrowserResourceDispatcherHostDelegate>
                            resource_dispatcher_host_delegate_;

  ExoBrowserMainParts*      browser_main_parts_;
  std::vector<ExoSession*>  sessions_;
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_CONTENT_BROWSER_CLIENT_H_
