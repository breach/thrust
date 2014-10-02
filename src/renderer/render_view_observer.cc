// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/render_view_observer.h"

#include "base/command_line.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebDraggableRegion.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_observer.h"

#include "src/common/switches.h"
#include "src/common/messages.h"

using namespace content;

namespace exo_shell {

ExoShellRenderViewObserver::ExoShellRenderViewObserver(
    RenderView* render_view)
    : RenderViewObserver(render_view) 
{
}

bool 
ExoShellRenderViewObserver::OnMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ExoShellRenderViewObserver, message)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void
ExoShellRenderViewObserver::DraggableRegionsChanged(
    blink::WebFrame* frame) 
{
  return;
}

}  // namespace exo_shell
