// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2014 GitHub, Inc.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_DIALOG_WEB_DIALOG_HELPER_H_
#define THRUST_SHELL_BROWSER_DIALOG_WEB_DIALOG_HELPER_H_

#include "base/memory/weak_ptr.h"

namespace base {
class FilePath;
}

namespace content {
struct FileChooserParams;
class WebContents;
}

namespace thrust_shell {

class ThrustWindow;

class ThrustShellWebDialogHelper {
 public:
  explicit ThrustShellWebDialogHelper(ThrustWindow* window);
  ~ThrustShellWebDialogHelper();

  void RunFileChooser(content::WebContents* web_contents,
                      const content::FileChooserParams& params);
  void EnumerateDirectory(content::WebContents* web_contents,
                          int request_id,
                          const base::FilePath& path);

 private:
  ThrustWindow* window_;

  base::WeakPtrFactory<ThrustShellWebDialogHelper> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellWebDialogHelper);
};

}  // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_DIALOG_WEB_DIALOG_HELPER_H_
