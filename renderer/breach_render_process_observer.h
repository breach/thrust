// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_RENDERER_BREACH_RENDER_PROCESS_OBSERVER_H_
#define BREACH_RENDERER_BREACH_RENDER_PROCESS_OBSERVER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/renderer/render_process_observer.h"
#include "ipc/ipc_platform_file.h"

namespace content {
class RenderView;
}

namespace breach {


class BreachRenderProcessObserver : public content::RenderProcessObserver {
 public:
  static BreachRenderProcessObserver* GetInstance();

  BreachRenderProcessObserver();
  virtual ~BreachRenderProcessObserver();

  void SetMainWindow(content::RenderView* view);

  // RenderProcessObserver implementation.
  virtual void WebKitInitialized() OVERRIDE;
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

 private:

  DISALLOW_COPY_AND_ASSIGN(BreachRenderProcessObserver);
};

} // namespace breach

#endif // BREACH_RENDERER_BREACH_RENDER_PROCESS_OBSERVER_H_
