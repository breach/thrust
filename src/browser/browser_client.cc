// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/browser_client.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "url/gurl.h"
#include "net/url_request/url_request_context_getter.h"
#include "ui/base/l10n/l10n_util.h"
#include "webkit/common/webpreferences.h"
#include "content/public/browser/browser_url_handler.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"

#include "src/browser/browser_main_parts.h"
#include "src/browser/resource_dispatcher_host_delegate.h"
#include "src/common/switches.h"
#include "src/browser/session/exo_session.h"
#include "src/geolocation/access_token_store.h"


using namespace content;

namespace exo_browser {

ExoBrowserBrowserClient::ExoBrowserBrowserClient()
{
}

ExoBrowserBrowserClient::~ExoBrowserBrowserClient() 
{
}

std::string 
ExoBrowserBrowserClient::GetApplicationLocale() {
  return l10n_util::GetApplicationLocale("");
}

void 
ExoBrowserBrowserClient::AppendExtraCommandLineSwitches(
    base::CommandLine* command_line,
    int child_process_id) 
{
  /*
  command_line->AppendSwitch(switches::kEnableThreadedCompositing);
#if defined(OS_MACOSX)
  command_line->AppendSwitch(switches::kUseCoreAnimation);
#endif
  */
}

void 
ExoBrowserBrowserClient::ResourceDispatcherHostCreated() 
{
  resource_dispatcher_host_delegate_.reset(
      new ExoBrowserResourceDispatcherHostDelegate());
  ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

AccessTokenStore* 
ExoBrowserBrowserClient::CreateAccessTokenStore()
{ 
  return new ExoBrowserAccessTokenStore();
}

std::string 
ExoBrowserBrowserClient::GetDefaultDownloadName() 
{
  return "download";
}


WebContentsViewDelegate* 
ExoBrowserBrowserClient::GetWebContentsViewDelegate(
    WebContents* web_contents) 
{ 
  return NULL;
  /* TODO(spolu): Reimplemenent with plugin */
  //return CreateExoBrowserWebContentsViewDelegate(web_contents);
}

void 
ExoBrowserBrowserClient::OverrideWebkitPrefs(
    RenderViewHost* render_view_host,
    const GURL& url,
    WebPreferences* prefs) 
{
  prefs->javascript_enabled = true;
  prefs->web_security_enabled = true;
  prefs->allow_file_access_from_file_urls = true;
  prefs->allow_universal_access_from_file_urls = true;
  prefs->allow_file_access_from_file_urls = true;
}

net::URLRequestContextGetter* 
ExoBrowserBrowserClient::CreateRequestContext(
    BrowserContext* content_browser_context,
    ProtocolHandlerMap* protocol_handlers,
    ProtocolHandlerScopedVector protocol_interceptors)
{
  ExoSession* session =
      ExoSessionForBrowserContext(content_browser_context);
  return session->CreateRequestContext(
      protocol_handlers, protocol_interceptors.Pass());
}

net::URLRequestContextGetter*
ExoBrowserBrowserClient::CreateRequestContextForStoragePartition(
    BrowserContext* content_browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers,
    ProtocolHandlerScopedVector protocol_interceptors)
{
  ExoSession* session =
      ExoSessionForBrowserContext(content_browser_context);
  return session->CreateRequestContextForStoragePartition(
      partition_path, in_memory, 
      protocol_handlers, protocol_interceptors.Pass());
}


bool 
ExoBrowserBrowserClient::IsHandledURL(
    const GURL& url) 
{
  if (!url.is_valid())
    return false;
  DCHECK_EQ(url.scheme(), StringToLowerASCII(url.scheme()));
  // Keep in sync with ProtocolHandlers added by
  // ExoBrowserURLRequestContextGetter::GetURLRequestContext().
  /* TODO(spolu): Check in sync */
  static const char* const kProtocolList[] = {
      kBlobScheme,
      kFileSystemScheme,
      kChromeUIScheme,
      kChromeDevToolsScheme,
      kDataScheme,
      kFileScheme,
  };
  for (size_t i = 0; i < arraysize(kProtocolList); ++i) {
    if (url.scheme() == kProtocolList[i])
      return true;
  }
  return false;
}

void 
ExoBrowserBrowserClient::RegisterExoSession(
    ExoSession* session)
{
  LOG(INFO) << "Register Session";
  sessions_.push_back(session);
}

void 
ExoBrowserBrowserClient::UnRegisterExoSession(
    ExoSession* session)
{
  std::vector<ExoSession*>::iterator it;
  for(it = sessions_.begin(); it != sessions_.end(); it++) {
    if(*it == session)
      break;
  }
  if(it != sessions_.end()) {
    LOG(INFO) << "UnRegister Session";
    sessions_.erase(it);
  }
}

ExoSession*
ExoBrowserBrowserClient::ExoSessionForBrowserContext(
    BrowserContext* browser_context) 
{
  std::vector<ExoSession*>::iterator it;
  for(it = sessions_.begin(); it != sessions_.end(); it++) {
    if(*it == browser_context) {
      return *it;
    }
  }
  return NULL;
}

ExoSession* 
ExoBrowserBrowserClient::system_session()
{
  return ((ExoSession*) browser_main_parts()->browser_context());
}

brightray::BrowserMainParts* 
ExoBrowserBrowserClient::OverrideCreateBrowserMainParts(
    const content::MainFunctionParams&) 
{
  return new ExoBrowserMainParts();
}

} // namespace exo_browser
