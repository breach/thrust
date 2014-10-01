// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_COMMON_MAIN_DELEGATE_H_
#define EXO_SHELL_COMMON_MAIN_DELEGATE_H_

#include "brightray/common/main_delegate.h"

namespace exo_shell {

class MainDelegate : public brightray::MainDelegate {
public:
  MainDelegate();
  ~MainDelegate();

private:
  virtual content::ContentBrowserClient* CreateContentBrowserClient() OVERRIDE;
  virtual content::ContentRendererClient* CreateContentRendererClient() OVERRIDE;

  scoped_ptr<content::ContentBrowserClient> browser_client_;
  scoped_ptr<content::ContentRendererClient> renderer_client_;

  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

}

#endif // EXO_SHELL_COMMON_MAIN_DELEGATE_H_
