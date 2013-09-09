// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UI_DIALOG_JAVASCRIPT_DIALOG_MANAGER_H_
#define EXO_BROWSER_BROWSER_UI_DIALOG_JAVASCRIPT_DIALOG_MANAGER_H_

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/javascript_dialog_manager.h"

namespace exo_browser {

class JavaScriptDialog;

class ExoBrowserJavaScriptDialogManager : 
    public content::JavaScriptDialogManager {
 public:
  ExoBrowserJavaScriptDialogManager();
  virtual ~ExoBrowserJavaScriptDialogManager();

  // JavaScriptDialogManager overrides
  virtual void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType javascript_message_type,
      const string16& message_text,
      const string16& default_prompt_text,
      bool user_gesture,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) OVERRIDE;

  virtual void RunBeforeUnloadDialog(
      content::WebContents* web_contents,
      const string16& message_text,
      bool is_reload,
      const DialogClosedCallback& callback) OVERRIDE;

  virtual void CancelActiveAndPendingDialogs(
      content::WebContents* web_contents) OVERRIDE;

  virtual void WebContentsDestroyed(
      content::WebContents* web_contents) OVERRIDE;

  // Called by the JavaScriptDialog when it closes.
  void DialogClosed(JavaScriptDialog* dialog);

  // Used for content_browsertests.
  void set_dialog_request_callback(const base::Closure& callback) {
    dialog_request_callback_ = callback;
  }

 private:
#if defined(OS_MACOSX) || defined(OS_WIN) || defined(TOOLKIT_GTK)
  // The dialog being shown. No queueing.
  scoped_ptr<JavaScriptDialog> dialog_;
#else
  /* TODO(spolu): implement JavaScriptDialog for other platforms, */
  /* and then drop this #if                                       */
#endif

  base::Closure dialog_request_callback_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserJavaScriptDialogManager);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_UI_DIALOG_JAVASCRIPT_DIALOG_MANAGER_H_
