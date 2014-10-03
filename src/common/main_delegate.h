// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_COMMON_MAIN_DELEGATE_H_
#define EXO_SHELL_COMMON_MAIN_DELEGATE_H_

#include "brightray/common/main_delegate.h"
#include "brightray/common/content_client.h"

namespace exo_shell {

class MainDelegate : public brightray::MainDelegate {
public:
  MainDelegate();
  ~MainDelegate();

protected:
  /****************************************************************************/
  /* CONTENT_MAIN_DELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual bool BasicStartupComplete(int* exit_code) OVERRIDE;
  virtual void PreSandboxStartup() OVERRIDE;
  virtual content::ContentBrowserClient* CreateContentBrowserClient() OVERRIDE;
  virtual content::ContentRendererClient*
      CreateContentRendererClient() OVERRIDE;

  /****************************************************************************/
  /* BRIGHTRAY_MAIN_DELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual scoped_ptr<brightray::ContentClient> CreateContentClient() OVERRIDE;
  virtual void AddDataPackFromPath(
      ui::ResourceBundle* bundle, const base::FilePath& pak_dir) OVERRIDE;
#if defined(OS_MACOSX)
  virtual void OverrideChildProcessPath() OVERRIDE;
  virtual void OverrideFrameworkBundlePath() OVERRIDE;
#endif

private:
  brightray::ContentClient content_client_;
  scoped_ptr<content::ContentBrowserClient> browser_client_;
  scoped_ptr<content::ContentRendererClient> renderer_client_;

  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

}

#endif // EXO_SHELL_COMMON_MAIN_DELEGATE_H_
