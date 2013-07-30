// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/renderer/breach_content_renderer_client.h"

#include "base/callback.h"
#include "base/command_line.h"
#include "base/debug/debugger.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/content_switches.h"
#include "content/public/renderer/render_view.h"
#include "content/public/test/layouttest_support.h"
#include "breach/common/breach_switches.h"
#include "breach/renderer/breach_render_process_observer.h"
#include "breach/renderer/breach_render_view_observer.h"
#include "v8/include/v8.h"

using namespace content;

namespace breach {

namespace {
BreachContentRendererClient* g_renderer_client;
}

BreachContentRendererClient* 
BreachContentRendererClient::Get() 
{
  return g_renderer_client;
}

BreachContentRendererClient::BreachContentRendererClient() 
{
  DCHECK(!g_renderer_client);
  g_renderer_client = this;
}

BreachContentRendererClient::~BreachContentRendererClient() 
{
  g_renderer_client = NULL;
}

void 
BreachContentRendererClient::RenderThreadStarted() 
{
  observer_.reset(new BreachRenderProcessObserver());
#if defined(OS_MACOSX)
  // We need to call this once before the sandbox was initialized to cache the
  // value.
  base::debug::BeingDebugged();
#endif
}

void 
BreachContentRendererClient::RenderViewCreated(
    RenderView* render_view) 
{
  new BreachRenderViewObserver(render_view);
}

}  // namespace breach
