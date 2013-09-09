// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/devtools/devtools_frontend.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "net/base/net_util.h"
#include "content/public/browser/devtools_http_handler.h"
#include "content/public/browser/devtools_manager.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/content_client.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/browser/browser_context.h"
#include "exo_browser/src/browser/browser_main_parts.h"
#include "exo_browser/src/browser/content_browser_client.h"
#include "exo_browser/src/browser/ui/exo_frame.h"
#include "exo_browser/src/devtools/devtools_delegate.h"

using namespace content;

namespace exo_browser {

namespace {

// DevTools frontend path for inspector LayoutTests.
GURL 
GetDevToolsPathAsURL() 
{
  base::FilePath dir_exe;
  if (!PathService::Get(base::DIR_EXE, &dir_exe)) {
    NOTREACHED();
    return GURL();
  }
#if defined(OS_MACOSX)
  // On Mac, the executable is in
  // out/Release/ExoBrowser.app/Contents/MacOS/ExoBrowser.
  // We need to go up 3 directories to get to out/Release.
  dir_exe = dir_exe.AppendASCII("../../..");
#endif
  base::FilePath dev_tools_path = dir_exe.AppendASCII(
      "resources/inspector/devtools.html");
  return net::FilePathToFileURL(dev_tools_path);
}

}  // namespace

// static
ExoBrowserDevToolsFrontend* 
ExoBrowserDevToolsFrontend::Show(
    WebContents* inspected_contents) {
  /*
  Browser* browser = Browser::CreateNewWindow(
      ExoBrowserContentBrowserClient::Get()->browser_context(),
      //inspected_contents->GetBrowserContext(),
      GURL(),
      NULL,
      MSG_ROUTING_NONE,
      gfx::Size());
      */
  /* TODO(spolU): Implement */
  ExoFrame *exo_frame = NULL;
  ExoBrowserDevToolsFrontend* devtools_frontend = 
    new ExoBrowserDevToolsFrontend(
      exo_frame,
      DevToolsAgentHost::GetOrCreateFor(
        inspected_contents->GetRenderViewHost()).get());

  ExoBrowserDevToolsDelegate* delegate = ExoBrowserContentBrowserClient::Get()->
    browser_main_parts()->devtools_delegate();
  LOG(INFO) << "DevTools: " 
            << delegate->devtools_http_handler()->GetFrontendURL(NULL);
  //browser->LoadURL(delegate->devtools_http_handler()->GetFrontendURL(NULL));

  return devtools_frontend;
}

void 
ExoBrowserDevToolsFrontend::Focus() 
{
  web_contents()->GetView()->Focus();
}

void 
ExoBrowserDevToolsFrontend::Close() 
{
  /* TODO(spolu) Implemebt / Fix */
  //exo_frame_->Kill();
}

ExoBrowserDevToolsFrontend::ExoBrowserDevToolsFrontend(
    ExoFrame* exo_frame,
    DevToolsAgentHost* agent_host)
/* TODO(spolu): Fix Implement */
  //: WebContentsObserver(exo_frame->web_contents()),
    : exo_frame_(exo_frame),
    agent_host_(agent_host) 
{
  frontend_host_.reset(
      DevToolsClientHost::CreateDevToolsFrontendHost(web_contents(), this));
}

ExoBrowserDevToolsFrontend::~ExoBrowserDevToolsFrontend() 
{
}

void 
ExoBrowserDevToolsFrontend::RenderViewCreated(
    RenderViewHost* render_view_host) 
{
  DevToolsClientHost::SetupDevToolsFrontendClient(
      web_contents()->GetRenderViewHost());
  DevToolsManager* manager = DevToolsManager::GetInstance();
  manager->RegisterDevToolsClientHostFor(agent_host_.get(),
                                         frontend_host_.get());
}

void 
ExoBrowserDevToolsFrontend::WebContentsDestroyed(
    WebContents* web_contents) 
{
  DevToolsManager::GetInstance()->ClientHostClosing(frontend_host_.get());
  delete this;
}

void 
ExoBrowserDevToolsFrontend::InspectedContentsClosing() 
{
  /* TODO(spolu) Implemebt / Fix */
  //exo_frame_->Kill();
}

} // namespace exo_browser
