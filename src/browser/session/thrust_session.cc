// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/session/thrust_session.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/threading/thread.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_util.h"
#include "net/base/escape.h"
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
#include "src/browser/web_view/web_view_guest.h"

using namespace content;

namespace thrust_shell {

/******************************************************************************/
/* RESOURCE CONTEXT */
/******************************************************************************/

class ThrustSession::ExoResourceContext : public content::ResourceContext {
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
      ThrustShellURLRequestContextGetter* getter) {
    getter_ = getter;
  }

 private:
  ThrustShellURLRequestContextGetter* getter_;

  DISALLOW_COPY_AND_ASSIGN(ExoResourceContext);
};

/******************************************************************************/
/* EXO SESSION */
/******************************************************************************/

ThrustSession::ThrustSession(
    ThrustSessionBinding* binding,
    const bool off_the_record,
    const std::string& path,
    bool dummy_cookie_store)
: binding_(binding),
  off_the_record_(off_the_record),
  ignore_certificate_errors_(false),
  resource_context_(new ExoResourceContext),
  cookie_store_(new ThrustSessionCookieStore(this, dummy_cookie_store)),
  visitedlink_store_(new ThrustSessionVisitedLinkStore(this)),
  current_instance_id_(0)
{
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }
#if defined(OS_WIN)
  std::wstring tmp(path.begin(), path.end());
  path_ = base::FilePath(tmp);
#else
  path_ = base::FilePath(path);
#endif

  visitedlink_store_->Init();

  devtools_delegate_ = new ThrustShellDevToolsDelegate(this);
  
  ThrustShellBrowserClient::Get()->RegisterThrustSession(this);
  LOG(INFO) << "ThrustSession Constructor " << this;
}


ThrustSession::~ThrustSession()
{
  /* If we're here that means that ou JS wrapper has been reclaimed */
  LOG(INFO) << "ThrustSession Destructor " << this;

  /* The ResourceContext is created on the UI thread but live son the IO */
  /* thread, so it must be deleted there.                                */
  if(resource_context_) {
    BrowserThread::DeleteSoon(
        BrowserThread::IO, FROM_HERE, resource_context_.release());
  }
  ThrustShellBrowserClient::Get()->UnRegisterThrustSession(this);
  /* We remove ourselves from the CookieStore as it may oulive us but we dont */
  /* want it to call into the API anymore.                                    */
  cookie_store_->parent_ = NULL;
  if(url_request_getter_.get())
    url_request_getter_.get()->parent_ = NULL;

  /* We also stop the DevToolsDelegate. It will destroy the delegate object. */
  if(devtools_delegate_)
    devtools_delegate_->Stop();
}

GURL
ThrustSession::GetDevToolsURL()
{
  return devtools_delegate_->devtools_http_handler()->GetFrontendURL();
}

base::FilePath 
ThrustSession::GetPath() const 
{
  return path_;
}

bool 
ThrustSession::IsOffTheRecord() const 
{
  return off_the_record_;
}


content::DownloadManagerDelegate* 
ThrustSession::GetDownloadManagerDelegate()  
{
  if (!download_manager_delegate_.get()) {
    DownloadManager* manager = BrowserContext::GetDownloadManager(this);
    download_manager_delegate_.reset(new ThrustShellDownloadManagerDelegate());
    download_manager_delegate_->SetDownloadManager(manager);
  }
  return download_manager_delegate_.get();
}

BrowserPluginGuestManager* 
ThrustSession::GetGuestManager() 
{
  LOG(INFO) << "************++++++++++++++++++ RETURN PLUGIN GUEST MANAGER";
  return this;
}

content::ResourceContext* 
ThrustSession::GetResourceContext()
{
  return resource_context_.get();
}


net::URLRequestContextGetter* 
ThrustSession::CreateRequestContext(
    ProtocolHandlerMap* protocol_handlers,
    URLRequestInterceptorScopedVector request_interceptors)
{
  DCHECK(!url_request_getter_.get());
  url_request_getter_ = new ThrustShellURLRequestContextGetter(
      this,
      ignore_certificate_errors_,
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers,
      request_interceptors.Pass(),
      ThrustShellMainParts::Get()->net_log());
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
ThrustSession::CreateRequestContextForStoragePartition(
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers,
    URLRequestInterceptorScopedVector request_interceptors)
{
  DCHECK(false);
  /* TODO(spolu): Add Support URLRequestContextGetter per StoragePartition. */
  /*              This might be made easier once http://crbug.com/159193    */
  /*              has landed in a release branch.                           */
  return NULL;
}

ThrustSessionCookieStore*
ThrustSession::GetCookieStore()
{
  return cookie_store_.get();
}

ThrustSessionVisitedLinkStore*
ThrustSession::GetVisitedLinkStore()
{
  return visitedlink_store_.get();
}

/******************************************************************************/
/* BROWSER_PLUGIN_GUEST_MANAGER */
/******************************************************************************/
void 
ThrustSession::MaybeGetGuestByInstanceIDOrKill(
    int guest_instance_id,
    int embedder_render_process_id,
    const GuestByInstanceIDCallback& callback) 
{
  content::WebContents* guest_web_contents =
      GetGuestByInstanceID(guest_instance_id, embedder_render_process_id);
  callback.Run(guest_web_contents);
}

WebContents* 
ThrustSession::GetGuestByInstanceID(
    int guest_instance_id,
    int embedder_render_process_id) 
{
  std::map<int, content::WebContents*>::const_iterator it =
      guest_web_contents_.find(guest_instance_id);
  if (it == guest_web_contents_.end())
    return NULL;
  return it->second;
}

bool 
ThrustSession::ForEachGuest(
    WebContents* embedder_web_contents,
    const GuestCallback& callback) 
{
  for (std::map<int, content::WebContents*>::iterator it =
           guest_web_contents_.begin();
       it != guest_web_contents_.end(); ++it) {
    WebContents* guest = it->second;
    WebViewGuest* guest_view = WebViewGuest::FromWebContents(guest);
    if (embedder_web_contents != guest_view->embedder_web_contents())
      continue;
    if (callback.Run(guest))
      return true;
  }
  return false;
}

void 
ThrustSession::AddGuest(
    int guest_instance_id,
    WebContents* guest_web_contents) 
{
  CHECK(!ContainsKey(guest_web_contents_, guest_instance_id));
  guest_web_contents_[guest_instance_id] = guest_web_contents;
}

void 
ThrustSession::RemoveGuest(
    int guest_instance_id) 
{
  std::map<int, content::WebContents*>::iterator it =
      guest_web_contents_.find(guest_instance_id);
  DCHECK(it != guest_web_contents_.end());
  guest_web_contents_.erase(it);
}

}  // namespace thrust_shell
