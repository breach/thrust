// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/renderer/render_view_observer.h"

#include "base/command_line.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebDraggableRegion.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_observer.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/common/messages.h"

using namespace content;

namespace exo_browser {

ExoBrowserRenderViewObserver::ExoBrowserRenderViewObserver(
    RenderView* render_view)
    : RenderViewObserver(render_view) 
{
}

bool 
ExoBrowserRenderViewObserver::OnMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(ExoBrowserRenderViewObserver, message)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

void 
ExoBrowserRenderViewObserver::DidClearWindowObject(
    blink::WebFrame* frame,
    int world_id) 
{
  return;
}

void
ExoBrowserRenderViewObserver::DraggableRegionsChanged(
    blink::WebFrame* frame) 
{
  return;
}

}  // namespace exo_browser
