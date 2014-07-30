// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/session/exo_session.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"
#include "content/public/browser/devtools_http_handler.h"

#include "src/common/switches.h"
#include "src/net/url_request_context_getter.h"
#include "src/browser/dialog/download_manager_delegate.h"
#include "src/browser/browser_main_parts.h"
#include "src/browser/browser_client.h"
#include "src/devtools/devtools_delegate.h"

using namespace content;

namespace exo_shell {

/******************************************************************************/
/*                             RESOURCE CONTEXT                               */
/******************************************************************************/

class ExoSession::ExoResourceContext : public content::ResourceContext {
 public:
  ExoResourceContext() 
    : getter_(NULL) {}
  virtual ~ExoResourceContext() {}

  // ResourceContext implementation:
  virtual net::HostResolver* GetHostResolver() OVERRIDE {
    CHECK(getter_);
    return getter_->host_resolver();
  }
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
    CHECK(getter_);
    return getter_->GetURLRequestContext();
  }
  virtual bool AllowMicAccess(const GURL& origin) OVERRIDE {
    return false;
  }
  virtual bool AllowCameraAccess(const GURL& origin) OVERRIDE {
    return false;
  }

  void set_url_request_context_getter(
      ExoShellURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  ExoShellURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(ExoResourceContext);
};

/******************************************************************************/
/*                               EXO SESSION                                  */
/******************************************************************************/

ExoSession::ExoSession(
    const bool off_the_record,
    const std::string& path,
    bool dummy_cookie_store)
: off_the_record_(off_the_record),
  ignore_certificate_errors_(false),
  resource_context_(new ExoResourceContext),
  cookie_store_(new ExoSessionCookieStore(this, dummy_cookie_store)),
  visitedlink_store_(new ExoSessionVisitedLinkStore(this))
{
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }
  path_ = base::FilePath(path);

  bool result = visitedlink_store_->Init();
  LOG(INFO) << "VisitedLink Init: " << result;

  devtools_delegate_ = new ExoShellDevToolsDelegate(this);
  
  ExoShellBrowserClient::Get()->RegisterExoSession(this);
}


ExoSession::~ExoSession()
{
  /* The ResourceContext is created on the UI thread but live son the IO */
  /* thread, so it must be deleted there.                                */
  if(resource_context_) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
  ExoShellBrowserClient::Get()->UnRegisterExoSession(this);
  /* We remove ourselves from the CookieStore as it may oulive us but we dont */
  /* want it to call into the API anymore.                                    */
  cookie_store_->parent_ = NULL;
  if(url_request_getter_.get())
    url_request_getter_.get()->parent_ = NULL;

  /* We also stop the DevToolsDelegate. It will destroy the delegate object. */
  if(devtools_delegate_)
    devtools_delegate_->Stop();

  /* If we're here that means that ou JS wrapper has been reclaimed */
  LOG(INFO) << "ExoSession Destructor";
}

GURL
ExoSession::GetDevToolsURL()
{
  return devtools_delegate_->devtools_http_handler()->GetFrontendURL();
}

base::FilePath 
ExoSession::GetPath() const 
{
  return path_;
}

bool 
ExoSession::IsOffTheRecord() const 
{
  return off_the_record_;
}


content::DownloadManagerDelegate* 
ExoSession::GetDownloadManagerDelegate()  
{
  if (!download_manager_delegate_.get()) {
    DownloadManager* manager = BrowserContext::GetDownloadManager(this);
    download_manager_delegate_.reset(new ExoShellDownloadManagerDelegate());
    download_manager_delegate_->SetDownloadManager(manager);
  }
  return download_manager_delegate_.get();
}


net::URLRequestContextGetter* 
ExoSession::GetRequestContext()
{
  /* TODO(spolu): We should not use GetDefaultStoragePartition, but this is */
  /*              apparently the simplest way waiting for                   */
  /*              http://crbug.com/159193 to land on release branches       */
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter* 
ExoSession::CreateRequestContext(
    ProtocolHandlerMap* protocol_handlers,
    ProtocolHandlerScopedVector protocol_interceptors)
{
  DCHECK(!url_request_getter_.get());
  url_request_getter_ = new ExoShellURLRequestContextGetter(
      this,
      ignore_certificate_errors_,
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers,
      protocol_interceptors.Pass(),
      ExoShellMainParts::Get()->net_log());
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
ExoSession::GetRequestContextForRenderProcess(
    int renderer_child_id)  
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoSession::GetMediaRequestContext()
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoSession::GetMediaRequestContextForRenderProcess(
    int renderer_child_id)
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoSession::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory)
{
  /* TODO(spolu): Check chrome. Note: used by geolocation */
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoSession::CreateRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers,
    ProtocolHandlerScopedVector protocol_interceptors)
{
  DCHECK(false);
  /* TODO(spolu): Add Support URLRequestContextGetter per StoragePartition. */
  /*              This might be made easier once http://crbug.com/159193    */
  /*              has landed in a release branch.                           */
  return NULL;
}



void 
ExoSession::RequestMidiSysExPermission(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    const GURL& requesting_frame,
    bool user_gesture,
    const MidiSysExPermissionCallback& callback) 
{
  callback.Run(false);
}


void 
ExoSession::CancelMidiSysExPermissionRequest(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    const GURL& requesting_frame)
{
}

void 
ExoSession::RequestProtectedMediaIdentifierPermission(
    int render_process_id,
    int render_view_id,
    int bridge_id,
    int group_id,
    const GURL& requesting_frame,
    const ProtectedMediaIdentifierPermissionCallback& callback) 
{
  callback.Run(true);
}

void 
ExoSession::CancelProtectedMediaIdentifierPermissionRequests(
    int group_id) 
{
}

quota::SpecialStoragePolicy* 
ExoSession::GetSpecialStoragePolicy() 
{
  return NULL;
}

GeolocationPermissionContext*
ExoSession::GetGeolocationPermissionContext()  
{
  return NULL;
}

content::ResourceContext* 
ExoSession::GetResourceContext()
{
  return resource_context_.get();
}

ExoSessionCookieStore*
ExoSession::GetCookieStore()
{
  return cookie_store_.get();
}

ExoSessionVisitedLinkStore*
ExoSession::GetVisitedLinkStore()
{
  return visitedlink_store_.get();
}

}  // namespace exo_shell
