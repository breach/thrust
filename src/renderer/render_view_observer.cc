// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/renderer/render_view_observer.h"

#include "base/command_line.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_observer.h"
#include "exo_browser/src/common/switches.h"

using namespace content;

namespace exo_browser {

ExoBrowserRenderViewObserver::ExoBrowserRenderViewObserver(
    RenderView* render_view)
    : RenderViewObserver(render_view) 
{
}

void 
ExoBrowserRenderViewObserver::DidClearWindowObject(
    blink::WebFrame* frame,
    int world_id) 
{
  return;
}

}  // namespace exo_browser
