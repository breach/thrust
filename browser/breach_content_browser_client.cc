// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/breach_content_browser_client.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "content/public/browser/browser_url_handler.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"

#include "breach/browser/breach_browser_main_parts.h"
#include "breach/browser/breach_browser_context.h"
#include "breach/browser/breach_resource_dispatcher_host_delegate.h"
#include "breach/browser/devtools/breach_devtools_delegate.h"
#include "breach/geolocation/breach_access_token_store.h"
#include "breach/common/breach_switches.h"
#include "breach/browser/ui/browser.h"

#include "net/url_request/url_request_context_getter.h"
#include "webkit/common/webpreferences.h"
#include "url/gurl.h"

using namespace content;

namespace breach {

namespace {

BreachContentBrowserClient* g_browser_client;

} // namespace

BreachContentBrowserClient* BreachContentBrowserClient::Get() {
  return g_browser_client;
}

BreachContentBrowserClient::BreachContentBrowserClient()
  : browser_main_parts_(NULL)
{
  DCHECK(!g_browser_client);
  g_browser_client = this;
}

BreachContentBrowserClient::~BreachContentBrowserClient() 
{
  g_browser_client = NULL;
}

BrowserMainParts* 
BreachContentBrowserClient::CreateBrowserMainParts(
    const MainFunctionParams& parameters) 
{
  browser_main_parts_ = new BreachBrowserMainParts(parameters);
  return browser_main_parts_;
}

void 
BreachContentBrowserClient::AppendExtraCommandLineSwitches(
    CommandLine* command_line,
    int child_process_id) 
{
}

void 
BreachContentBrowserClient::ResourceDispatcherHostCreated() 
{
  resource_dispatcher_host_delegate_.reset(
      new BreachResourceDispatcherHostDelegate());
  ResourceDispatcherHost::Get()->SetDelegate(
      resource_dispatcher_host_delegate_.get());
}

std::string 
BreachContentBrowserClient::GetDefaultDownloadName() 
{
  return "download";
}

void 
BreachContentBrowserClient::BrowserURLHandlerCreated(
    BrowserURLHandler* handler) 
{
}

BreachBrowserContext* 
BreachContentBrowserClient::browser_context() 
{
  return browser_main_parts_->browser_context();
}

BreachBrowserContext*
BreachContentBrowserClient::off_the_record_browser_context() 
{
  return browser_main_parts_->off_the_record_browser_context();
}

AccessTokenStore* 
BreachContentBrowserClient::CreateAccessTokenStore() 
{
  return new BreachAccessTokenStore(browser_context());
}

void 
BreachContentBrowserClient::OverrideWebkitPrefs(
    RenderViewHost* render_view_host,
    const GURL& url,
    WebPreferences* prefs) 
{
  /* TODO(spolu): check */
  // Disable web security.
  //prefs->dom_paste_enabled = true;
  //prefs->javascript_can_access_clipboard = true;
  //prefs->web_security_enabled = true;
  //prefs->allow_file_access_from_file_urls = true;

  // Open experimental features.
  prefs->css_sticky_position_enabled = true;
  prefs->css_shaders_enabled = true;
  prefs->css_variables_enabled = true;

  // Disable plugins and cache by default.
  prefs->plugins_enabled = false;
  prefs->java_enabled = false;
}

net::URLRequestContextGetter* 
BreachContentBrowserClient::CreateRequestContext(
    BrowserContext* content_browser_context,
    ProtocolHandlerMap* protocol_handlers) 
{
  BreachBrowserContext* browser_context =
      BreachBrowserContextForBrowserContext(content_browser_context);
  return browser_context->CreateRequestContext(protocol_handlers);
}

net::URLRequestContextGetter*
BreachContentBrowserClient::CreateRequestContextForStoragePartition(
    BrowserContext* content_browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers) 
{
  BreachBrowserContext* browser_context =
    BreachBrowserContextForBrowserContext(content_browser_context);
  return browser_context->CreateRequestContextForStoragePartition(
      partition_path, in_memory, protocol_handlers);
}


BreachBrowserContext*
BreachContentBrowserClient::BreachBrowserContextForBrowserContext(
    BrowserContext* content_browser_context) 
{
  if (content_browser_context == browser_context())
    return browser_context();
  DCHECK_EQ(content_browser_context, off_the_record_browser_context());
  return off_the_record_browser_context();
}

bool 
BreachContentBrowserClient::IsHandledURL(
    const GURL& url) 
{
  if (!url.is_valid())
    return false;
  DCHECK_EQ(url.scheme(), StringToLowerASCII(url.scheme()));
  // Keep in sync with ProtocolHandlers added by
  // BreachURLRequestContextGetter::GetURLRequestContext().
  /* TODO(spolu): Check in sync */
  static const char* const kProtocolList[] = {
      chrome::kBlobScheme,
      chrome::kFileSystemScheme,
      chrome::kChromeUIScheme,
      chrome::kChromeDevToolsScheme,
      chrome::kDataScheme,
      chrome::kFileScheme,
  };
  for (size_t i = 0; i < arraysize(kProtocolList); ++i) {
    if (url.scheme() == kProtocolList[i])
      return true;
  }
  return false;
}

void 
BreachContentBrowserClient::RenderProcessHostCreated(
    content::RenderProcessHost* host) 
{
}

}  // namespace breach
