// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/common/main_delegate.h"

#include "src/browser/browser_client.h"
#include "src/renderer/renderer_client.h"

namespace exo_browser {

MainDelegate::MainDelegate() {
}

MainDelegate::~MainDelegate() {
}

content::ContentBrowserClient* 
MainDelegate::CreateContentBrowserClient() {
  browser_client_.reset(new ExoBrowserBrowserClient);
  return browser_client_.get();
}

content::ContentRendererClient* 
MainDelegate::CreateContentRendererClient() {
  renderer_client_.reset(new ExoBrowserRendererClient);
  return renderer_client_.get();
}

}
