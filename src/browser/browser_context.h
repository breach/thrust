// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors. 
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_BROWSER_CONTEXT_H_
#define EXO_BROWSER_BROWSER_BROWSER_CONTEXT_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/content_browser_client.h"
#include "net/url_request/url_request_job_factory.h"

namespace net {
class NetLog;
}

namespace exo_browser {

class DownloadManagerDelegate;
class ResourceContext;
class ExoBrowserURLRequestContextGetter;
class ExoBrowserDownloadManagerDelegate;

class ExoBrowserContext : public content::BrowserContext {
 public:
  ExoBrowserContext(bool off_the_record, 
                    net::NetLog* net_log);
  virtual ~ExoBrowserContext();

  // BrowserContext implementation.
  virtual base::FilePath GetPath() const OVERRIDE;
  virtual bool IsOffTheRecord() const OVERRIDE;
  virtual content::DownloadManagerDelegate* 
    GetDownloadManagerDelegate() OVERRIDE;

  virtual net::URLRequestContextGetter* GetRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContext() OVERRIDE;
  virtual net::URLRequestContextGetter* GetMediaRequestContextForRenderProcess(
      int renderer_child_id) OVERRIDE;
  virtual net::URLRequestContextGetter*
      GetMediaRequestContextForStoragePartition(
          const base::FilePath& partition_path,
          bool in_memory) OVERRIDE;
  virtual void RequestMIDISysExPermission(
      int render_process_id,
      int render_view_id,
      const GURL& requesting_frame,
      const MIDISysExPermissionCallback& callback) OVERRIDE;
  virtual content::GeolocationPermissionContext*
      GetGeolocationPermissionContext() OVERRIDE;
  virtual quota::SpecialStoragePolicy* GetSpecialStoragePolicy() OVERRIDE;

  virtual content::ResourceContext* GetResourceContext() OVERRIDE;

  net::URLRequestContextGetter* CreateRequestContext(
      content::ProtocolHandlerMap* protocol_handlers);
  net::URLRequestContextGetter* CreateRequestContextForStoragePartition(
      const base::FilePath& partition_path,
      bool in_memory,
      content::ProtocolHandlerMap* protocol_handlers);

 private:
  class ExoBrowserResourceContext;

  /* Performs initialization of the ExoBrowserContext while IO is */
  /* still allowed on the current thread.                                */
  void InitWhileIOAllowed();

  bool off_the_record_;
  net::NetLog* net_log_;
  bool ignore_certificate_errors_;
  base::FilePath path_;

  scoped_ptr<ExoBrowserResourceContext> resource_context_;
  scoped_refptr<ExoBrowserURLRequestContextGetter> url_request_getter_;
  scoped_refptr<ExoBrowserDownloadManagerDelegate> download_manager_delegate_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserContext);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_BROWSER_CONTEXT_H_
