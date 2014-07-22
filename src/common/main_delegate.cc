// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "common/main_delegate.h"

#include "browser/browser_client.h"
#include "renderer/renderer_client.h"

namespace exo_browser {

MainDelegate::MainDelegate() {
}

MainDelegate::~MainDelegate() {
}

content::ContentBrowserClient* MainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new BrowserClient);
  return browser_client_.get();
}

content::ContentRendererClient* MainDelegate::CreateContentRendererClient() {
  renderer_client_.reset(new RendererClient);
  return renderer_client_.get();
}

}
