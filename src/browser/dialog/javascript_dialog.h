// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UI_DIALOG_JAVASCRIPT_DIALOG_H_
#define EXO_BROWSER_BROWSER_UI_DIALOG_JAVASCRIPT_DIALOG_H_

#include "content/public/browser/javascript_dialog_manager.h"

#if defined(TOOLKIT_GTK)
#include "ui/base/gtk/gtk_signal.h"
#endif

#if defined(OS_MACOSX)
#if __OBJC__
@class JavaScriptDialogHelper;
#else
class JavaScriptDialogHelper;
#endif  // __OBJC__
#endif  // defined(OS_MACOSX)

namespace exo_browser {

class ExoBrowserJavaScriptDialogManager;

class JavaScriptDialog {
 public:
  JavaScriptDialog(
      ExoBrowserJavaScriptDialogManager* manager,
      gfx::NativeWindow parent_window,
      content::JavaScriptMessageType message_type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const content::JavaScriptDialogManager::DialogClosedCallback& callback);
  ~JavaScriptDialog();

  // Called to cancel a dialog mid-flight.
  void Cancel();

 private:
  ExoBrowserJavaScriptDialogManager* manager_;
  content::JavaScriptDialogManager::DialogClosedCallback callback_;

#if defined(OS_MACOSX)
  JavaScriptDialogHelper* helper_;  // owned
#elif defined(OS_WIN)
  content::JavaScriptMessageType message_type_;
  HWND dialog_win_;
  base::string16 message_text_;
  base::string16 default_prompt_text_;
  static INT_PTR CALLBACK DialogProc(HWND dialog, UINT message, WPARAM wparam,
                                     LPARAM lparam);
#elif defined(TOOLKIT_GTK)
  GtkWidget* gtk_dialog_;
  gfx::NativeWindow parent_window_;
  CHROMEGTK_CALLBACK_1(JavaScriptDialog, void, OnResponse, int);
#endif

  DISALLOW_COPY_AND_ASSIGN(JavaScriptDialog);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_UI_DIALOG_JAVASCRIPT_DIALOG_H_
