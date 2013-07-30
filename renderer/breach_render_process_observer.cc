// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/renderer/breach_render_process_observer.h"

#include "base/command_line.h"
#include "content/public/common/content_client.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "content/public/test/layouttest_support.h"
#include "breach/common/breach_messages.h"
#include "breach/common/breach_switches.h"
#include "breach/renderer/breach_content_renderer_client.h"

using namespace content;

namespace breach {

namespace {
BreachRenderProcessObserver* g_instance = NULL;
}

// static
BreachRenderProcessObserver* 
BreachRenderProcessObserver::GetInstance() 
{
  return g_instance;
}

BreachRenderProcessObserver::BreachRenderProcessObserver()
{
  CHECK(!g_instance);
  g_instance = this;
  RenderThread::Get()->AddObserver(this);
}

BreachRenderProcessObserver::~BreachRenderProcessObserver() 
{
  CHECK(g_instance == this);
  g_instance = NULL;
}

void 
BreachRenderProcessObserver::SetMainWindow(
    RenderView* view) 
{
  return;
}

void 
BreachRenderProcessObserver::WebKitInitialized() 
{
  return;
}

bool 
BreachRenderProcessObserver::OnControlMessageReceived(
    const IPC::Message& message) 
{
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(BreachRenderProcessObserver, message)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()

  return handled;
}

} // namespace breach
