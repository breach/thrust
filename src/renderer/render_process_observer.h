// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_RENDERER_RENDER_PROCESS_OBSERVER_H_
#define THRUST_SHELL_RENDERER_RENDER_PROCESS_OBSERVER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "ipc/ipc_platform_file.h"
#include "content/public/renderer/render_process_observer.h"

namespace content {
class RenderView;
}

namespace thrust_shell {

class ThrustShellRenderProcessObserver : public content::RenderProcessObserver {
 public:
  static ThrustShellRenderProcessObserver* GetInstance();

  ThrustShellRenderProcessObserver();
  virtual ~ThrustShellRenderProcessObserver();

  /****************************************************************************/
  /* RENDERPROCESSOBSERVER IMPLEMENTATIO */
  /****************************************************************************/
  virtual void WebKitInitialized() OVERRIDE;
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  void EnableWebRuntimeFeatures();

  DISALLOW_COPY_AND_ASSIGN(ThrustShellRenderProcessObserver);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_RENDERER_RENDER_PROCESS_OBSERVER_H_
