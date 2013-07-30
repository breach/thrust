// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/renderer/breach_render_view_observer.h"

#include "base/command_line.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/render_view_observer.h"
#include "breach/common/breach_switches.h"
#include "third_party/WebKit/public/web/WebView.h"

using namespace content;

namespace breach {

BreachRenderViewObserver::BreachRenderViewObserver(
    RenderView* render_view)
    : RenderViewObserver(render_view) 
{
}

void 
BreachRenderViewObserver::DidClearWindowObject(WebKit::WebFrame* frame) 
{
  return;
}

}  // namespace breach
