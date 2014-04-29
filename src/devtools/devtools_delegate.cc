// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/devtools/devtools_delegate.h"

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
#include "grit/exo_browser_resources.h"
#include "net/socket/tcp_listen_socket.h"
#include "ui/base/resource/resource_bundle.h"
#include "exo_browser/src/browser/exo_frame.h"

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
  virtual std::string GetType() const OVERRIDE { return kTargetTypePage; }
  virtual std::string GetTitle() const OVERRIDE { return title_; }
  virtual std::string GetDescription() const OVERRIDE { return std::string(); }
  virtual GURL GetUrl() const OVERRIDE { return url_; }
  virtual GURL GetFaviconUrl() const OVERRIDE { return favicon_url_; }
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
  agent_host_ =
      DevToolsAgentHost::GetOrCreateFor(web_contents->GetRenderViewHost());
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
  RenderViewHost* rvh = agent_host_->GetRenderViewHost();
  if (!rvh)
    return false;
  WebContents* web_contents = WebContents::FromRenderViewHost(rvh);
  if (!web_contents)
    return false;
  web_contents->GetDelegate()->ActivateContents(web_contents);
  return true;
}

bool 
Target::Close() const 
{
  RenderViewHost* rvh = agent_host_->GetRenderViewHost();
  if (!rvh)
    return false;
  rvh->ClosePage();
  return true;
}


}  // namespace

namespace exo_browser {

ExoBrowserDevToolsDelegate::ExoBrowserDevToolsDelegate(ExoSession* session)
: session_(session)
{
  LOG(INFO) << "DEVTOOLS DELEGATE CREATED";
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

scoped_ptr<DevToolsTarget>
ExoBrowserDevToolsDelegate::CreateNewTarget(const GURL& url) {
  return scoped_ptr<DevToolsTarget>();
}

void 
ExoBrowserDevToolsDelegate::EnumerateTargets(TargetCallback callback) {
  TargetList targets;
  std::vector<RenderViewHost*> rvh_list =
    content::DevToolsAgentHost::GetValidRenderViewHosts();
  for(std::vector<RenderViewHost*>::iterator it = rvh_list.begin();
      it != rvh_list.end(); ++it) {
    WebContents* web_contents = WebContents::FromRenderViewHost(*it);
    if(web_contents) {
      ExoFrame* frame = ExoFrame::ExoFrameForWebContents(web_contents);
      if(frame->session() == session_) {
        targets.push_back(new Target(web_contents));
      }
    }
  }
  callback.Run(targets);
}


scoped_ptr<net::StreamListenSocket>
ExoBrowserDevToolsDelegate::CreateSocketForTethering(
    net::StreamListenSocket::Delegate* delegate,
    std::string* name) {
  return scoped_ptr<net::StreamListenSocket>(); 
}

} // namespace exo_browser
