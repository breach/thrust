// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/devtools/devtools_delegate.h"

#include <vector>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/strings/string_number_conversions.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "grit/exo_browser_resources.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"

using namespace content;

namespace {

net::StreamListenSocketFactory* CreateSocketFactory() {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  // See if the user specified a port on the command line (useful for
  // automation). If not, use an ephemeral port by specifying 0.
  int port = 0;
  if(command_line.HasSwitch(switches::kRemoteDebuggingPort)) {
    int temp_port;
    std::string port_str =
        command_line.GetSwitchValueASCII(switches::kRemoteDebuggingPort);
    if(base::StringToInt(port_str, &temp_port) &&
       temp_port > 0 && temp_port < 65535) {
      port = temp_port;
    } 
    else {
      DLOG(WARNING) << "Invalid http debugger port number " << temp_port;
    }
  }
  return new net::TCPListenSocketFactory("127.0.0.1", port);
}

}  // namespace

namespace exo_browser {

ExoBrowserDevToolsDelegate::ExoBrowserDevToolsDelegate(
    BrowserContext* browser_context)
  : browser_context_(browser_context) 
{
  /* TODO(spolu): Sort out that comment */
  // Note that Content Shell always used bundled DevTools frontend,
  // even on Android, because the shell is used for running layout tests.
  devtools_http_handler_ =
    DevToolsHttpHandler::Start(CreateSocketFactory(), std::string(), this);
}

ExoBrowserDevToolsDelegate::~ExoBrowserDevToolsDelegate() 
{
}

void 
ExoBrowserDevToolsDelegate::Stop() 
{
  // The call below destroys this.
  devtools_http_handler_->Stop();
}

std::string 
ExoBrowserDevToolsDelegate::GetDiscoveryPageHTML() 
{
  return ResourceBundle::GetSharedInstance().GetRawDataResource(
      IDR_EXO_BROWSER_DEVTOOLS_DISCOVERY_PAGE).as_string();
}

bool 
ExoBrowserDevToolsDelegate::BundlesFrontendResources() 
{
  return true;
}

base::FilePath 
ExoBrowserDevToolsDelegate::GetDebugFrontendDir() 
{
  return base::FilePath();
}

std::string 
ExoBrowserDevToolsDelegate::GetPageThumbnailData(const GURL& url) 
{
  return std::string();
}

RenderViewHost* 
ExoBrowserDevToolsDelegate::CreateNewTarget() {
  return NULL;
  /* TODO(spolu): fix */
  /*
  Browser* browser = ExoBrowser::CreateNewWindow(browser_context_,
                                              GURL(kAboutBlankURL),
                                              NULL,
                                              MSG_ROUTING_NONE,
                                              gfx::Size());
  return browser->web_contents()->GetRenderViewHost();
  */
}

DevToolsHttpHandlerDelegate::TargetType
ExoBrowserDevToolsDelegate::GetTargetType(RenderViewHost*) {
  return kTargetTypeTab;
}

std::string ExoBrowserDevToolsDelegate::GetViewDescription(
    content::RenderViewHost*) {
  return std::string();
}

scoped_refptr<net::StreamListenSocket>
ExoBrowserDevToolsDelegate::CreateSocketForTethering(
    net::StreamListenSocket::Delegate* delegate,
    std::string* name) {
  return NULL;
}

} // namespace exo_browser
