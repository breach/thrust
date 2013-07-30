// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/breach_browser_context.h"

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

#include "breach/common/breach_switches.h"
#include "breach/browser/breach_url_request_context_getter.h"
#include "breach/browser/breach_download_manager_delegate.h"

#if defined(OS_WIN)
#include "base/base_paths_win.h"
#elif defined(OS_LINUX)
#include "base/nix/xdg_util.h"
#elif defined(OS_MACOSX)
#include "base/base_paths_mac.h"
#endif

using namespace content;

namespace breach {

class BreachBrowserContext::BreachResourceContext : public ResourceContext {
 public:
  BreachResourceContext() 
    : getter_(NULL) {}
  virtual ~BreachResourceContext() {}

  // ResourceContext implementation:
  virtual net::HostResolver* GetHostResolver() OVERRIDE {
    CHECK(getter_);
    return getter_->host_resolver();
  }
  virtual net::URLRequestContext* GetRequestContext() OVERRIDE {
    CHECK(getter_);
    return getter_->GetURLRequestContext();
  }

  void set_url_request_context_getter(BreachURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  BreachURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(BreachResourceContext);
};



BreachBrowserContext::BreachBrowserContext(
    bool off_the_record,
    net::NetLog* net_log)
: off_the_record_(off_the_record),
  net_log_(net_log),
  ignore_certificate_errors_(false),
  resource_context_(new BreachResourceContext) 
{
  InitWhileIOAllowed();
}

BreachBrowserContext::~BreachBrowserContext() 
{
  if (resource_context_) {
    BrowserThread::DeleteSoon(
      BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
}

void 
BreachBrowserContext::InitWhileIOAllowed() 
{
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }
  if (cmd_line->HasSwitch(switches::kBreachDataPath)) {
    path_ = cmd_line->GetSwitchValuePath(switches::kBreachDataPath);
    return;
  }
#if defined(OS_WIN)
  CHECK(PathService::Get(base::DIR_LOCAL_APP_DATA, &path_));
  path_ = path_.Append(std::wstring(L"breach"));
#elif defined(OS_LINUX)
  scoped_ptr<base::Environment> env(base::Environment::Create());
  base::FilePath config_dir(
      base::nix::GetXDGDirectory(env.get(),
                                 base::nix::kXdgConfigHomeEnvVar,
                                 base::nix::kDotConfigDir));
  path_ = config_dir.Append("breach");
#elif defined(OS_MACOSX)
  CHECK(PathService::Get(base::DIR_APP_DATA, &path_));
  path_ = path_.Append("Breach");
#else
  NOTIMPLEMENTED();
#endif

  if (!base::PathExists(path_))
    file_util::CreateDirectory(path_);
}

base::FilePath 
BreachBrowserContext::GetPath() const 
{
  return path_;
}

bool 
BreachBrowserContext::IsOffTheRecord() const 
{
  return off_the_record_;
}

DownloadManagerDelegate* 
BreachBrowserContext::GetDownloadManagerDelegate()  
{
  DownloadManager* manager = BrowserContext::GetDownloadManager(this);
  if (!download_manager_delegate_.get()) {
    download_manager_delegate_ = new BreachDownloadManagerDelegate();
    download_manager_delegate_->SetDownloadManager(manager);
  }
  return download_manager_delegate_.get();
}

net::URLRequestContextGetter* 
BreachBrowserContext::GetRequestContext()
{
  return GetDefaultStoragePartition(this)->GetURLRequestContext();
}

net::URLRequestContextGetter* 
BreachBrowserContext::CreateRequestContext(
    ProtocolHandlerMap* protocol_handlers) 
{
  DCHECK(!url_request_getter_.get());
  url_request_getter_ = new BreachURLRequestContextGetter(
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
BreachBrowserContext::GetRequestContextForRenderProcess(
    int renderer_child_id)  
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
BreachBrowserContext::GetMediaRequestContext()
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
BreachBrowserContext::GetMediaRequestContextForRenderProcess(
    int renderer_child_id)
{
  return GetRequestContext();
}

net::URLRequestContextGetter*
BreachBrowserContext::GetMediaRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory)
{
  /* TODO(spolu): Check chrome. Note: used by geolocation */
  return GetRequestContext();
}

net::URLRequestContextGetter*
BreachBrowserContext::CreateRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers) 
{
  return NULL;
}

ResourceContext* 
BreachBrowserContext::GetResourceContext()
{
  return resource_context_.get();
}

GeolocationPermissionContext*
BreachBrowserContext::GetGeolocationPermissionContext()
{
  return NULL;
}

quota::SpecialStoragePolicy* 
BreachBrowserContext::GetSpecialStoragePolicy()
{
  return NULL;
}

}  // namespace breach
