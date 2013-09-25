// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "exo_browser/src/browser/session/exo_session.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/resource_context.h"
#include "content/public/browser/storage_partition.h"
#include "content/public/common/content_switches.h"

#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/net/url_request_context_getter.h"
#include "exo_browser/src/browser/download_manager_delegate.h"
#include "exo_browser/src/browser/browser_main_parts.h"
#include "exo_browser/src/browser/content_browser_client.h"
#include "exo_browser/src/node/api/exo_session_wrap.h"

using namespace content;

namespace exo_browser {

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
      ExoBrowserURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  ExoBrowserURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(ExoResourceContext);
};

/******************************************************************************/
/*                               EXO SESSION                                  */
/******************************************************************************/

ExoSession::ExoSession(
    const bool off_the_record,
    const std::string& path,
    ExoSessionWrap* wrapper)
: off_the_record_(off_the_record),
  ignore_certificate_errors_(false),
  resource_context_(new ExoResourceContext),
  wrapper_(wrapper)
{
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }
  if (cmd_line->HasSwitch(switches::kExoBrowserDataPath)) {
    path_ = cmd_line->GetSwitchValuePath(switches::kExoBrowserDataPath);
    return;
  }
  else {
    path_ = base::FilePath(path);
  }

  ExoBrowserContentBrowserClient::Get()->RegisterExoSession(this);

  /* As this can be overrided by the CommandLine, let's make sure it exists. */
  if(!base::PathExists(path_))
    file_util::CreateDirectory(path_);
}


ExoSession::~ExoSession()
{
  /* The ResourceContext is created on the UI thread but live son the IO */
  /* thread, so it must be deleted there.                                */
  if(resource_context_) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
  ExoBrowserContentBrowserClient::Get()->UnRegisterExoSession(this);

  /* If we're here that means that ou JS wrapper has been reclaimed */
  LOG(INFO) << "ExoSesion Destructor";
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
    ProtocolHandlerMap* protocol_handlers) 
{
  DCHECK(!url_request_getter_.get());
  url_request_getter_ = new ExoBrowserURLRequestContextGetter(
      ignore_certificate_errors_,
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers,
      ExoBrowserContentBrowserClient::Get()->browser_main_parts()->net_log());
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
    ProtocolHandlerMap* protocol_handlers) 
{
  /* TODO(spolu): Add Support URLRequestContextGetter per StoragePartition. */
  /*              This might be made easier once http://crbug.com/159193    */
  /*              has landed in a release branch.                           */
  return NULL;
}



content::DownloadManagerDelegate* 
ExoSession::GetDownloadManagerDelegate()  
{
  if (!download_manager_delegate_.get()) {
    DownloadManager* manager = BrowserContext::GetDownloadManager(this);
    download_manager_delegate_ = new ExoBrowserDownloadManagerDelegate();
    download_manager_delegate_->SetDownloadManager(manager);
  }
  return download_manager_delegate_.get();
}

void 
ExoSession::RequestMIDISysExPermission(
    int render_process_id,
    int render_view_id,
    const GURL& requesting_frame,
    const MIDISysExPermissionCallback& callback) 
{
  callback.Run(false);
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

}  // namespace exo_browser
