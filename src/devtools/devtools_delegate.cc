// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/devtools/devtools_delegate.h"

#include <vector>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_target.h"
#include "content/public/browser/favicon_status.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"

#include "src/browser/browser_client.h"
#include "src/browser/session/thrust_session.h"

using namespace content;

namespace {

const char kTargetTypePage[] = "page";

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


class Target : public content::DevToolsTarget {
 public:
  explicit Target(WebContents* web_contents);

  virtual std::string GetId() const OVERRIDE { return id_; }
  virtual std::string GetParentId() const OVERRIDE { return std::string(); }
  virtual std::string GetType() const OVERRIDE { return kTargetTypePage; }
  virtual std::string GetTitle() const OVERRIDE { return title_; }
  virtual std::string GetDescription() const OVERRIDE { return std::string(); }
  virtual GURL GetURL() const OVERRIDE { return url_; }
  virtual GURL GetFaviconURL() const OVERRIDE { return favicon_url_; }
  virtual base::TimeTicks GetLastActivityTime() const OVERRIDE {
    return last_activity_time_;
  }
  virtual bool IsAttached() const OVERRIDE {
    return agent_host_->IsAttached();
  }
  virtual scoped_refptr<DevToolsAgentHost> GetAgentHost() const OVERRIDE {
    return agent_host_;
  }
  virtual bool Activate() const OVERRIDE;
  virtual bool Close() const OVERRIDE;

 private:
  scoped_refptr<DevToolsAgentHost> agent_host_;
  std::string id_;
  std::string title_;
  GURL url_;
  GURL favicon_url_;
  base::TimeTicks last_activity_time_;
};

Target::Target(
    WebContents* web_contents) 
{
  agent_host_ = DevToolsAgentHost::GetOrCreateFor(web_contents);
  id_ = agent_host_->GetId();
  title_ = base::UTF16ToUTF8(web_contents->GetTitle());
  url_ = web_contents->GetURL();
  content::NavigationController& controller = web_contents->GetController();
  content::NavigationEntry* entry = controller.GetActiveEntry();
  if (entry != NULL && entry->GetURL().is_valid())
    favicon_url_ = entry->GetFavicon().url;
  last_activity_time_ = web_contents->GetLastActiveTime();
}
 
bool 
Target::Activate() const 
{
  WebContents* web_contents = agent_host_->GetWebContents();
  if(!web_contents) {
    return false;
  }
  web_contents->GetDelegate()->ActivateContents(web_contents);
  return true;
}

bool 
Target::Close() const 
{
  WebContents* web_contents = agent_host_->GetWebContents();
  if(!web_contents) {
    return false;
  }
  web_contents->GetRenderViewHost()->ClosePage();
  return true;
}


}  // namespace

namespace thrust_shell {

ThrustShellDevToolsDelegate::ThrustShellDevToolsDelegate(ThrustSession* session)
: session_(session)
{
  LOG(INFO) << "ThrustShellDevToolsDelegate Constructor";
  devtools_http_handler_ =
    DevToolsHttpHandler::Start(CreateSocketFactory(), std::string(), this,
                               base::FilePath());
}

ThrustShellDevToolsDelegate::~ThrustShellDevToolsDelegate() 
{
  LOG(INFO) << "ThrustShellDevToolsDelegate Destructor";
}

void
ThrustShellDevToolsDelegate::Stop()
{
  LOG(INFO) << "ThrustShellDevToolsDelegate Stop";
  /* This call destroys this delegates. Be carefule double destroy. */
  devtools_http_handler_->Stop();
}

std::string 
ThrustShellDevToolsDelegate::GetDiscoveryPageHTML() 
{
  return std::string("<html></html>");
}

bool 
ThrustShellDevToolsDelegate::BundlesFrontendResources() 
{
  return true;
}

base::FilePath 
ThrustShellDevToolsDelegate::GetDebugFrontendDir() 
{
  return base::FilePath();
}

std::string 
ThrustShellDevToolsDelegate::GetPageThumbnailData(const GURL& url) 
{
  return std::string();
}

scoped_ptr<DevToolsTarget>
ThrustShellDevToolsDelegate::CreateNewTarget(const GURL& url) {
  return scoped_ptr<DevToolsTarget>();
}

void 
ThrustShellDevToolsDelegate::EnumerateTargets(TargetCallback callback) {
  TargetList targets;
  std::vector<WebContents*> wc_list =
      content::DevToolsAgentHost::GetInspectableWebContents();
  for (std::vector<WebContents*>::iterator it = wc_list.begin();
       it != wc_list.end();
       ++it) {
    /* We push only if this WebContents is part of this session. */
    /* TODO(spolu) Check this filtering is working. */
    if(ThrustShellBrowserClient::Get()
        ->ThrustSessionForBrowserContext(
          (*it)->GetBrowserContext()) == session_) {
      targets.push_back(new Target(*it));
    }
  }
  callback.Run(targets);
}


scoped_ptr<net::StreamListenSocket>
ThrustShellDevToolsDelegate::CreateSocketForTethering(
    net::StreamListenSocket::Delegate* delegate,
    std::string* name) {
  return scoped_ptr<net::StreamListenSocket>(); 
}

} // namespace thrust_shell
