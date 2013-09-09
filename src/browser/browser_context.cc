// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/browser_context.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/environment.h"
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

#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX)
#include "base/base_paths_mac.h"
#endif

using namespace content;

namespace exo_browser {

class ExoBrowserContext::ExoBrowserResourceContext : 
  public content::ResourceContext {
 public:
  ExoBrowserResourceContext() 
    : getter_(NULL) {}
  virtual ~ExoBrowserResourceContext() {}

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

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserResourceContext);
};



ExoBrowserContext::ExoBrowserContext(
    bool off_the_record,
    net::NetLog* net_log)
: off_the_record_(off_the_record),
  net_log_(net_log),
  ignore_certificate_errors_(false),
  resource_context_(new ExoBrowserResourceContext) 
{
  InitWhileIOAllowed();
}

ExoBrowserContext::~ExoBrowserContext() 
{
  if (resource_context_) {
    BrowserThread::DeleteSoon(
      BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
}

void 
ExoBrowserContext::InitWhileIOAllowed() 
{
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }
  if (cmd_line->HasSwitch(switches::kExoBrowserDataPath)) {
    path_ = cmd_line->GetSwitchValuePath(switches::kExoBrowserDataPath);
    return;
  }
#if defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &path_));
  path_ = path_.Append(std::wstring(L"exo_browser"));
#elif defined(OS_LINUX)
  scoped_ptr<base::Environment> env(base::Environment::Create());
  base::FilePath config_dir(
      base::nix::GetXDGDirectory(env.get(),
                                 base::nix::kXdgConfigHomeEnvVar,
                                 base::nix::kDotConfigDir));
  path_ = config_dir.Append("exo_browser");
#elif defined(OS_MACOSX)
  CHECK(PathService::Get(base::DIR_APP_DATA, &path_));
  path_ = path_.Append("ExoBrowser");
#else
  NOTIMPLEMENTED();
#endif

  if (!base::PathExists(path_))
    file_util::CreateDirectory(path_);
}

base::FilePath 
ExoBrowserContext::GetPath() const 
{
  return path_;
}

bool 
ExoBrowserContext::IsOffTheRecord() const 
{
  return off_the_record_;
}

content::DownloadManagerDelegate* 
ExoBrowserContext::GetDownloadManagerDelegate()  
{
  DownloadManager* manager = BrowserContext::GetDownloadManager(this);
  if (!download_manager_delegate_.get()) {
    download_manager_delegate_ = new ExoBrowserDownloadManagerDelegate();
    download_manager_delegate_->SetDownloadManager(manager);
  }
  return download_manager_delegate_.get();
}

net::URLRequestContextGetter* 
ExoBrowserContext::GetRequestContext()
{
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter* 
ExoBrowserContext::CreateRequestContext(
    ProtocolHandlerMap* protocol_handlers) 
{
  DCHECK(!url_request_getter_.get());
  url_request_getter_ = new ExoBrowserURLRequestContextGetter(
      ignore_certificate_errors_,
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers,
      net_log_);
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
ExoBrowserContext::GetRequestContextForRenderProcess(
    int renderer_child_id)  
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoBrowserContext::GetMediaRequestContext()
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoBrowserContext::GetMediaRequestContextForRenderProcess(
    int renderer_child_id)
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoBrowserContext::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory)
{
  /* TODO(spolu): Check chrome. Note: used by geolocation */
  return GetRequestContext();
}

net::URLRequestContextGetter*
ExoBrowserContext::CreateRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers) 
{
  return NULL;
}

content::ResourceContext* 
ExoBrowserContext::GetResourceContext()
{
  return resource_context_.get();
}

void 
ExoBrowserContext::RequestMIDISysExPermission(
    int render_process_id,
    int render_view_id,
    const GURL& requesting_frame,
    const MIDISysExPermissionCallback& callback) 
{
  callback.Run(false);
}

quota::SpecialStoragePolicy* 
ExoBrowserContext::GetSpecialStoragePolicy() 
{
  return NULL;
}

GeolocationPermissionContext*
ExoBrowserContext::GetGeolocationPermissionContext()  
{
  return NULL;
}

}  // namespace exo_browser
