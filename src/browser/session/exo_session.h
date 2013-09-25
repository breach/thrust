// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_H_
#define EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_H_

#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "net/url_request/url_request_job_factory.h"

namespace exo_browser {

class ExoSessionWrap;
class DownloadManagerDelegate;
class ResourceContext;
class ExoBrowserURLRequestContextGetter;
class ExoBrowserDownloadManagerDelegate;

// ### ExoSession
//
// The ExoSession is BrowserContext passed to an ExoBrowser to be used with all
// its associated control and frames. A session represents the contextual 
// parameters and data needed to render a web page:
// - Whether or not Local HTML5 Storage is allowed or in-memory
// - An interface to retrieve cookies
// - An interface to retrieve visited links
//
// These interfaces are exposed to Javascript. So that user profiles can be
// handled from there. HTML5 Storage is not exposed to Javascript for the sake
// of simplicity, but we do allow to force it into memory (use of public
// computer)
//
// TODO(spolu): This discrepancy between cookies and local storage may lead to 
// issues as some website may expect to find local storage given the presence of 
// a cookie. Exposing HTML5 Storage to JS is on a future "distant?" roadmap...
// see http://www.w3.org/TR/webstorage/
// - SessionStorage not impacted
// - LocalStorage can be expired / deleted by user so semantics are not too off
//
// The ExoSession initialization comes from Javascript. It is aware of its
// associated JS wrapper (used to call handlers, dispatch callbacks, ...)
//
// The ExoSession lives on the BrowserThread::UI thread, and should therefore
// PostTask on the NodeJS thread to communicate with its wrapper.
class ExoSession : public content::BrowserContext {
public:
  /****************************************************************************/
  /*                           PUBLIC INTERFACE                               */
  /****************************************************************************/
  // ### ~ExoSession
  virtual ~ExoSession();

  /****************************************************************************/
  /*                     BROWSER CONTEXT IMPLEMENTATION                       */
  /****************************************************************************/
  virtual base::FilePath GetPath() const OVERRIDE;
  virtual bool IsOffTheRecord() const OVERRIDE;

  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) OVERRIDE;

  virtual content::DownloadManagerDelegate* 
    GetDownloadManagerDelegate() OVERRIDE;

  virtual void RequestMIDISysExPermission(
      int render_process_id,
      int render_view_id,
      const GURL& requesting_frame,
      const MIDISysExPermissionCallback& callback) OVERRIDE;
  virtual content::GeolocationPermissionContext*
      GetGeolocationPermissionContext() OVERRIDE;
  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;

  virtual content::ResourceContext* GetResourceContext() OVERRIDE;

  /****************************************************************************/
  /*                     REQUEST CONTEXT GETTER HELPERS                       */
  /****************************************************************************/
  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers);
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers);

private:
  class ExoResourceContext;

  /****************************************************************************/
  /*                           PRIVATE INTERFACE                              */
  /****************************************************************************/
  // ### ExoSession
  ExoSession(bool off_the_record,
             std::string& path,
             ExoSessionWrap* wrapper = NULL);

  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  bool                                             off_the_record_;
  bool                                             ignore_certificate_errors_;
  base::FilePath                                   path_;

  scoped_ptr<ExoResourceContext>                   resource_context_;
  scoped_refptr<ExoBrowserURLRequestContextGetter> url_request_getter_;
  scoped_refptr<ExoBrowserDownloadManagerDelegate> download_manager_delegate_;

  ExoSessionWrap*                                  wrapper_;

  friend class ExoSessionWrap;

  DISALLOW_COPY_AND_ASSIGN(ExoSession);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_H_
