// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BREACH_MAIN_DELEGATE_H_
#define BREACH_BREACH_MAIN_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/app/content_main_delegate.h"
/* TODO(spolu): create BreachContentClient and all common files */
#include "content/shell/common/shell_content_client.h"

namespace breach {

class BreachContentBrowserClient;
/* TODO(spolu): renaming post file creation */
class ShellContentRendererClient;

class BreachMainDelegate : public content::ContentMainDelegate {
 public:
  BreachMainDelegate();
  virtual ~BreachMainDelegate();

  // ContentMainDelegate implementation:
  virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
  virtual void PreSandboxStartup() OVERRIDE;
  virtual int RunProcess(
      const std::string& process_type,
      const MainFunctionParams& main_function_params) OVERRIDE;
  virtual ContentBrowserClient* CreateContentBrowserClient() OVERRIDE;
  virtual ContentRendererClient* CreateContentRendererClient() OVERRIDE;

  static void InitializeResourceBundle();

 private:
  scoped_ptr<BreachContentBrowserClient> browser_client_;
  /* TODO(spolu): renaming post file creation */
  scoped_ptr<ShellContentRendererClient> renderer_client_;
  ShellContentClient content_client_;

  DISALLOW_COPY_AND_ASSIGN(BreachMainDelegate);
};

}  // namespace breach

#endif  // BREACH_BREACH_MAIN_DELEGATE_H_
