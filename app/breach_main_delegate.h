// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BREACH_MAIN_DELEGATE_H_
#define BREACH_BREACH_MAIN_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/app/content_main_delegate.h"
#include "breach/common/breach_content_client.h"

namespace breach {

class BreachContentBrowserClient;
class BreachContentRendererClient;

class BreachMainDelegate : public content::ContentMainDelegate {
 public:
  BreachMainDelegate();
  virtual ~BreachMainDelegate();

  // ContentMainDelegate implementation:
  virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
  virtual void PreSandboxStartup() OVERRIDE;
  virtual content::ContentBrowserClient* 
    CreateContentBrowserClient() OVERRIDE;
  virtual content::ContentRendererClient* 
    CreateContentRendererClient() OVERRIDE;

  static void InitializeResourceBundle();

 private:
  scoped_ptr<BreachContentBrowserClient> browser_client_;
  scoped_ptr<BreachContentRendererClient> renderer_client_;
  BreachContentClient content_client_;

  DISALLOW_COPY_AND_ASSIGN(BreachMainDelegate);
};

}  // namespace breach

#endif  // BREACH_BREACH_MAIN_DELEGATE_H_
