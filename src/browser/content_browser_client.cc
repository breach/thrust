// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/content_browser_client.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "url/gurl.h"
#include "net/url_request/url_request_context_getter.h"
#include "webkit/common/webpreferences.h"
#include "content/public/browser/browser_url_handler.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"
#include "exo_browser/src/browser/browser_main_parts.h"
#include "exo_browser/src/browser/resource_dispatcher_host_delegate.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/browser/web_contents_view_delegate_creator.h"
#include "exo_browser/src/browser/session/exo_session.h"


using namespace content;

namespace exo_browser {

namespace {

ExoBrowserContentBrowserClient* g_browser_client;

} // namespace

ExoBrowserContentBrowserClient* 
ExoBrowserContentBrowserClient::Get() 
{
  return g_browser_client;
}

ExoBrowserContentBrowserClient::ExoBrowserContentBrowserClient()
  : browser_main_parts_(NULL)
{
  DCHECK(!g_browser_client);
  g_browser_client = this;
}

ExoBrowserContentBrowserClient::~ExoBrowserContentBrowserClient() 
{
  g_browser_client = NULL;
}

BrowserMainParts* 
ExoBrowserContentBrowserClient::CreateBrowserMainParts(
    const MainFunctionParams& parameters) 
{
  browser_main_parts_ = new ExoBrowserMainParts(parameters);
  return browser_main_parts_;
}

void 
ExoBrowserContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line,
    int child_process_id) 
{
}

void 
ExoBrowserContentBrowserClient::ResourceDispatcherHostCreated() 
{
  resource_dispatcher_host_delegate_.reset(
      new ExoBrowserResourceDispatcherHostDelegate());
  ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

std::string 
ExoBrowserContentBrowserClient::GetDefaultDownloadName() 
{
  return "download";
}


WebContentsViewDelegate* 
ExoBrowserContentBrowserClient::GetWebContentsViewDelegate(
    WebContents* web_contents) 
{ 
#if !defined(USE_AURA)
  return CreateExoBrowserWebContentsViewDelegate(web_contents);
#else
  return NULL;
#endif
}

void 
ExoBrowserContentBrowserClient::BrowserURLHandlerCreated(
    BrowserURLHandler* handler) 
{
}

void 
ExoBrowserContentBrowserClient::OverrideWebkitPrefs(
    RenderViewHost* render_view_host,
    const GURL& url,
    WebPreferences* prefs) 
{
}

net::URLRequestContextGetter* 
ExoBrowserContentBrowserClient::CreateRequestContext(
    BrowserContext* content_browser_context,
    ProtocolHandlerMap* protocol_handlers) 
{
  ExoSession* session =
      ExoSessionForBrowserContext(content_browser_context);
  return session->CreateRequestContext(protocol_handlers);
}

net::URLRequestContextGetter*
ExoBrowserContentBrowserClient::CreateRequestContextForStoragePartition(
    BrowserContext* content_browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers) 
{
  ExoSession* session =
      ExoSessionForBrowserContext(content_browser_context);
  return session->CreateRequestContextForStoragePartition(
      partition_path, in_memory, protocol_handlers);
}


bool 
ExoBrowserContentBrowserClient::IsHandledURL(
    const GURL& url) 
{
  if (!url.is_valid())
    return false;
  DCHECK_EQ(url.scheme(), StringToLowerASCII(url.scheme()));
  // Keep in sync with ProtocolHandlers added by
  // ExoBrowserURLRequestContextGetter::GetURLRequestContext().
  /* TODO(spolu): Check in sync */
  static const char* const kProtocolList[] = {
      chrome::kBlobScheme,
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
ExoBrowserContentBrowserClient::RegisterExoSession(
    ExoSession* session)
{
  LOG(INFO) << "Register Session";
  sessions_.push_back(session);
}

void 
ExoBrowserContentBrowserClient::UnRegisterExoSession(
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
ExoBrowserContentBrowserClient::ExoSessionForBrowserContext(
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


} // namespace exo_browser
