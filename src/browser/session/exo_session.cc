// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/session/exo_session.h"

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

namespace exo_shell {

/******************************************************************************/
/* RESOURCE CONTEXT */
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
/* EXO SESSION */
/******************************************************************************/

ExoSession::ExoSession(
    const bool off_the_record,
    const std::string& path,
    bool dummy_cookie_store)
: off_the_record_(off_the_record),
  ignore_certificate_errors_(false),
  resource_context_(new ExoResourceContext),
  cookie_store_(new ExoSessionCookieStore(this, dummy_cookie_store)),
  visitedlink_store_(new ExoSessionVisitedLinkStore(this)),
  current_instance_id_(0)
{
  CommandLine* cmd_line = CommandLine::ForCurrentProcess();
  if (cmd_line->HasSwitch(switches::kIgnoreCertificateErrors)) {
    ignore_certificate_errors_ = true;
  }
  path_ = base::FilePath(path);

  bool result = visitedlink_store_->Init();

  devtools_delegate_ = new ExoShellDevToolsDelegate(this);
  
  ExoShellBrowserClient::Get()->RegisterExoSession(this);
  LOG(INFO) << "ExoSession Constructor " << this;
}


ExoSession::~ExoSession()
{
  /* If we're here that means that ou JS wrapper has been reclaimed */
  LOG(INFO) << "ExoSession Destructor " << this;

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

BrowserPluginGuestManager* 
ExoSession::GetGuestManager() 
{
  LOG(INFO) << "************++++++++++++++++++ RETURN PLUGIN GUEST MANAGER";
  return this;
}

content::ResourceContext* 
ExoSession::GetResourceContext()
{
  return resource_context_.get();
}


net::URLRequestContextGetter* 
ExoSession::CreateRequestContext(
    ProtocolHandlerMap* protocol_handlers,
    URLRequestInterceptorScopedVector request_interceptors)
{
  DCHECK(!url_request_getter_.get());
  url_request_getter_ = new ExoShellURLRequestContextGetter(
      this,
      ignore_certificate_errors_,
      GetPath(),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO),
      BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE),
      protocol_handlers,
      request_interceptors.Pass(),
      ExoShellMainParts::Get()->net_log());
  resource_context_->set_url_request_context_getter(url_request_getter_.get());
  return url_request_getter_.get();
}

net::URLRequestContextGetter*
ExoSession::CreateRequestContextForStoragePartition(
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

/******************************************************************************/
/* BROWSER_PLUGIN_GUEST_MANAGER */
/******************************************************************************/

WebContents* 
ExoSession::CreateGuest(
    SiteInstance* embedder_site_instance,
    int instance_id,
    scoped_ptr<base::DictionaryValue> extra_params) 
{
  LOG(INFO) << "CREATE GUEST ***************";

  std::string storage_partition_id;
  bool persist_storage = false;
  std::string storage_partition_string;
  WebViewGuest::ParsePartitionParam(
      extra_params.get(), &storage_partition_id, &persist_storage);

  content::RenderProcessHost* embedder_process_host =
      embedder_site_instance->GetProcess();
  // Validate that the partition id coming from the renderer is valid UTF-8,
  // since we depend on this in other parts of the code, such as FilePath
  // creation. 
  if (!base::IsStringUTF8(storage_partition_id)) {
    return NULL;
  }

  const GURL& embedder_site_url = embedder_site_instance->GetSiteURL();
  const std::string& host = embedder_site_url.host();

  /* TODO(spolu): Reintroduce guest_instance_id when site-isolation is live   */
  /* with <webview>. See /src/chrome/browser/guest_view/guest_view_manager.cc */
  /*
  std::string url_encoded_partition = net::EscapeQueryParamValue(
      storage_partition_id, false);
  // The SiteInstance of a given webview tag is based on the fact that it's
  // a guest process in addition to which platform application the tag
  // belongs to and what storage partition is in use, rather than the URL
  // that the tag is being navigated to.
  GURL guest_site(base::StringPrintf("%s://%s/%s?%s",
                                     content::kGuestScheme,
                                     host.c_str(),
                                     persist_storage ? "persist" : "",
                                     url_encoded_partition.c_str()));

  // If we already have a webview tag in the same app using the same storage
  // partition, we should use the same SiteInstance so the existing tag and
  // the new tag can script each other.
  SiteInstance* guest_site_instance = GetGuestSiteInstance(guest_site);
  if (!guest_site_instance) {
    // Create the SiteInstance in a new BrowsingInstance, which will ensure
    // that webview tags are also not allowed to send messages across
    // different partitions.
    guest_site_instance = SiteInstance::CreateForURL(
        embedder_site_instance->GetBrowserContext(), guest_site);
  }
  */

  WebContents::CreateParams create_params((BrowserContext*)this);
  create_params.guest_instance_id = instance_id;
  create_params.guest_extra_params.reset(extra_params.release());
  return WebContents::Create(create_params);
}

int 
ExoSession::GetNextInstanceID() 
{
  return ++current_instance_id_;
}

void 
ExoSession::MaybeGetGuestByInstanceIDOrKill(
    int guest_instance_id,
    int embedder_render_process_id,
    const GuestByInstanceIDCallback& callback) 
{
  content::WebContents* guest_web_contents =
      GetGuestByInstanceID(guest_instance_id, embedder_render_process_id);
  callback.Run(guest_web_contents);
}

WebContents* 
ExoSession::GetGuestByInstanceID(
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
ExoSession::ForEachGuest(
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
ExoSession::AddGuest(
    int guest_instance_id,
    WebContents* guest_web_contents) 
{
  CHECK(!ContainsKey(guest_web_contents_, guest_instance_id));
  guest_web_contents_[guest_instance_id] = guest_web_contents;
}

void 
ExoSession::RemoveGuest(
    int guest_instance_id) 
{
  std::map<int, content::WebContents*>::iterator it =
      guest_web_contents_.find(guest_instance_id);
  DCHECK(it != guest_web_contents_.end());
  guest_web_contents_.erase(it);
}

}  // namespace exo_shell
