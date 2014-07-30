// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_RENDERER_RENDER_PROCESS_OBSERVER_H_
#define EXO_SHELL_RENDERER_RENDER_PROCESS_OBSERVER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "ipc/ipc_platform_file.h"
#include "content/public/renderer/render_process_observer.h"

namespace content {
class RenderView;
}

namespace exo_shell {

class ExoShellRenderProcessObserver : public content::RenderProcessObserver {
 public:
  static ExoShellRenderProcessObserver* GetInstance();

  ExoShellRenderProcessObserver();
  virtual ~ExoShellRenderProcessObserver();

  void SetMainWindow(content::RenderView* view);

  // RenderProcessObserver implementation.
  virtual void WebKitInitialized() OVERRIDE;
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:

  DISALLOW_COPY_AND_ASSIGN(ExoShellRenderProcessObserver);
};

} // namespace exo_shell

#endif // EXO_SHELL_RENDERER_RENDER_PROCESS_OBSERVER_H_
