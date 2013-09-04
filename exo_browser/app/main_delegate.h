// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_APP_MAIN_DELEGATE_H_
#define EXO_BROWSER_APP_MAIN_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/app/content_main_delegate.h"
#include "exo/exo_browser/common/content_client.h"

namespace exo_browser {

class ExoBrowserContentBrowserClient;
class ExoBrowserContentRendererClient;

class ExoBrowserMainDelegate : public content::ContentMainDelegate {
 public:
  ExoBrowserMainDelegate();
  virtual ~ExoBrowserMainDelegate();

  // ContentMainDelegate implementation:
  virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
  virtual void PreSandboxStartup() OVERRIDE;
  virtual content::ContentBrowserClient* 
    CreateContentBrowserClient() OVERRIDE;
  virtual content::ContentRendererClient* 
    CreateContentRendererClient() OVERRIDE;

  static void InitializeResourceBundle();

 private:
  scoped_ptr<ExoBrowserContentBrowserClient> browser_client_;
  scoped_ptr<ExoBrowserContentRendererClient> renderer_client_;
  ExoBrowserContentClient content_client_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserMainDelegate);
};

} // namespace exo_browser

#endif // EXO_BROWSER_APP_MAIN_DELEGATE_H_
