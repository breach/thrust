// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_RENDERER_RENDER_PROCESS_OBSERVER_H_
#define EXO_BROWSER_RENDERER_RENDER_PROCESS_OBSERVER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "ipc/ipc_platform_file.h"
#include "content/public/renderer/render_process_observer.h"

namespace content {
class RenderView;
}

namespace exo_browser {

class ExoBrowserRenderProcessObserver : public content::RenderProcessObserver {
 public:
  static ExoBrowserRenderProcessObserver* GetInstance();

  ExoBrowserRenderProcessObserver();
  virtual ~ExoBrowserRenderProcessObserver();

  void SetMainWindow(content::RenderView* view);

  // RenderProcessObserver implementation.
  virtual void WebKitInitialized() OVERRIDE;
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserRenderProcessObserver);
};

} // namespace exo_browser

#endif // EXO_BROWSER_RENDERER_RENDER_PROCESS_OBSERVER_H_
