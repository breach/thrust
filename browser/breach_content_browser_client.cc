// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/app/breach_content_browser_client.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/file_util.h"
#include "base/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "content/browser/child_process_security_policy_impl.h"
#include "content/public/browser/browser_url_handler.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/resource_dispatcher_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/common/url_constants.h"

/* TODO(spolu): renaming post file creation */
#include "content/nw/src/api/dispatcher_host.h"
#include "content/nw/src/common/shell_switches.h"
#include "content/nw/src/browser/printing/print_job_manager.h"
#include "content/nw/src/browser/shell_devtools_delegate.h"
#include "content/nw/src/browser/shell_resource_dispatcher_host_delegate.h"
#include "content/nw/src/nw_shell.h"
#include "content/nw/src/nw_version.h"
#include "content/nw/src/shell_browser_context.h"
#include "content/nw/src/shell_browser_main_parts.h"

#include "geolocation/shell_access_token_store.h"
#include "googleurl/src/gurl.h"
#include "ui/base/l10n/l10n_util.h"
#include "webkit/dom_storage/dom_storage_map.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/user_agent/user_agent_util.h"
#include "webkit/plugins/npapi/plugin_list.h"

using namespace content;

namespace breach {

BreachContentBrowserClient::BreachContentBrowserClient()
  : browser_main_parts_(NULL),
    master_rph_(NULL) 
{
}

BreachContentBrowserClient::~BreachContentBrowserClient() 
{
}

BrowserMainParts* 
BreachContentBrowserClient::CreateBrowserMainParts(
    const MainFunctionParams& parameters) 
{
  /* TODO(spolu): renaming post file creation */
  browser_main_parts_ = new ShellBrowserMainParts(parameters);
  return browser_main_parts_;
}

std::string 
BreachContentBrowserClient::GetApplicationLocale() 
{
  return l10n_util::GetApplicationLocale("en-US");
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
  /* TODO(spolu): renaming post file creation */
      new ShellResourceDispatcherHostDelegate());
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

/* TODO(spolu): renaming post file creation */
ShellBrowserContext* 
BreachContentBrowserClient::browser_context() 
{
  return browser_main_parts_->browser_context();
}

/* TODO(spolu): renaming post file creation */
ShellBrowserContext*
ShellContentBrowserClient::off_the_record_browser_context() 
{
  return browser_main_parts_->off_the_record_browser_context();
}

AccessTokenStore* 
ShellContentBrowserClient::CreateAccessTokenStore() 
{
  return new ShellAccessTokenStore(browser_context());
}

void 
ShellContentBrowserClient::OverrideWebkitPrefs(
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

bool 
ShellContentBrowserClient::ShouldTryToUseExistingProcessHost(
      BrowserContext* browser_context, 
      const GURL& url) 
{
  ShellBrowserContext* shell_browser_context =
    static_cast<ShellBrowserContext*>(browser_context);
  if (shell_browser_context->pinning_renderer())
    return true;
  else
    return false;
}

bool 
ShellContentBrowserClient::IsSuitableHost(
    RenderProcessHost* process_host,
    const GURL& site_url) 
{
  return process_host == master_rph_;
}

net::URLRequestContextGetter* 
ShellContentBrowserClient::CreateRequestContext(
    BrowserContext* content_browser_context,
    ProtocolHandlerMap* protocol_handlers) 
{
  ShellBrowserContext* shell_browser_context =
      ShellBrowserContextForBrowserContext(content_browser_context);
  return shell_browser_context->CreateRequestContext(protocol_handlers);
}

net::URLRequestContextGetter*
ShellContentBrowserClient::CreateRequestContextForStoragePartition(
    BrowserContext* content_browser_context,
    const base::FilePath& partition_path,
    bool in_memory,
    ProtocolHandlerMap* protocol_handlers) 
{
  ShellBrowserContext* shell_browser_context =
    ShellBrowserContextForBrowserContext(content_browser_context);
  return shell_browser_context->CreateRequestContextForStoragePartition(
      partition_path, in_memory, protocol_handlers);
}


ShellBrowserContext*
ShellContentBrowserClient::ShellBrowserContextForBrowserContext(
    BrowserContext* content_browser_context) 
{
  if (content_browser_context == browser_context())
    return browser_context();
  DCHECK_EQ(content_browser_context, off_the_record_browser_context());
  return off_the_record_browser_context();
}

void 
ShellContentBrowserClient::RenderProcessHostCreated(
    RenderProcessHost* host) 
{
  int id = host->GetID();
  if (!master_rph_)
    master_rph_ = host;
  // Grant file: scheme to the whole process, since we impose
  // per-view access checks.
  content::ChildProcessSecurityPolicy::GetInstance()->GrantScheme(
      host->GetID(), chrome::kFileScheme);

#if defined(ENABLE_PRINTING)
  host->GetChannel()->AddFilter(new PrintingMessageFilter(id));
#endif
}

}  // namespace content
