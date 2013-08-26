// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_BREACH_DEVTOOLS_FRONTEND_H_
#define BREACH_BROWSER_BREACH_DEVTOOLS_FRONTEND_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/devtools_agent_host.h"
#include "content/public/browser/devtools_client_host.h"
#include "content/public/browser/devtools_frontend_host_delegate.h"
#include "content/public/browser/web_contents_observer.h"

namespace content {
class RenderViewHost;
class WebContents;
}


namespace breach {
class ExoFrame;

class BreachDevToolsFrontend : public content::WebContentsObserver,
                               public content::DevToolsFrontendHostDelegate {
 public:
  static BreachDevToolsFrontend* Show(
      content::WebContents* inspected_contents);
  void Focus();
  void Close();

  ExoFrame* exo_frame() const { return exo_frame_; }

 private:
  BreachDevToolsFrontend(
      ExoFrame* exo_frame, 
      content::DevToolsAgentHost* agent_host);
  virtual ~BreachDevToolsFrontend();

  // WebContentsObserver overrides
  virtual void RenderViewCreated(
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void WebContentsDestroyed(
      content::WebContents* web_contents) OVERRIDE;

  // DevToolsFrontendHostDelegate implementation
  virtual void ActivateWindow() OVERRIDE {}
  virtual void ChangeAttachedWindowHeight(unsigned height) OVERRIDE {}
  virtual void CloseWindow() OVERRIDE {}
  virtual void MoveWindow(int x, int y) OVERRIDE {}
  virtual void SetDockSide(const std::string& side) OVERRIDE {}
  virtual void OpenInNewTab(const std::string& url) OVERRIDE {}
  virtual void SaveToFile(const std::string& url,
                          const std::string& content,
                          bool save_as) OVERRIDE {}
  virtual void AppendToFile(const std::string& url,
                            const std::string& content) OVERRIDE {}
  virtual void RequestFileSystems() OVERRIDE {}
  virtual void AddFileSystem() OVERRIDE {}
  virtual void RemoveFileSystem(const std::string& file_system_path) OVERRIDE {}
  virtual void IndexPath(int request_id,
                         const std::string& file_system_path) OVERRIDE {}
  virtual void StopIndexing(int request_id) OVERRIDE {}
  virtual void SearchInPath(int request_id,
                            const std::string& file_system_path,
                            const std::string& query) OVERRIDE {}

  virtual void InspectedContentsClosing() OVERRIDE;

  ExoFrame* exo_frame_;
  scoped_refptr<content::DevToolsAgentHost> agent_host_;
  scoped_ptr<content::DevToolsClientHost> frontend_host_;

  DISALLOW_COPY_AND_ASSIGN(BreachDevToolsFrontend);
};

}  // namespace breach

#endif // BREACH_BROWSER_BREACH_DEVTOOLS_FRONTEND_H_
