// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/dialog/javascript_dialog.h"

#include "base/strings/string_util.h"
#include "exo_browser/src/app/resource.h"
#include "exo_browser/src/browser/exo_browser.h"
#include "exo_browser/src/browser/dialog/javascript_dialog_manager.h"

using namespace content;

namespace exo_browser {

class JavaScriptDialog;

INT_PTR CALLBACK 
JavaScriptDialog::DialogProc(HWND dialog,
                             UINT message,
                             WPARAM wparam,
                             LPARAM lparam) 
{
  switch (message) {
    case WM_INITDIALOG: {
      SetWindowLongPtr(dialog, DWLP_USER, static_cast<LONG_PTR>(lparam));
      JavaScriptDialog* owner =
          reinterpret_cast<JavaScriptDialog*>(lparam);
      owner->dialog_win_ = dialog;
      SetDlgItemText(dialog, IDC_DIALOGTEXT, owner->message_text_.c_str());
      if (owner->message_type_ == JAVASCRIPT_MESSAGE_TYPE_PROMPT)
        SetDlgItemText(dialog, IDC_PROMPTEDIT,
                       owner->default_prompt_text_.c_str());
      break;
    }
    case WM_DESTROY: {
      JavaScriptDialog* owner = reinterpret_cast<JavaScriptDialog*>(
          GetWindowLongPtr(dialog, DWLP_USER));
      if (owner->dialog_win_) {
        owner->dialog_win_ = 0;
        owner->callback_.Run(false, string16());
        owner->manager_->DialogClosed(owner);
      }
      break;
    }
    case WM_COMMAND: {
      JavaScriptDialog* owner = reinterpret_cast<JavaScriptDialog*>(
          GetWindowLongPtr(dialog, DWLP_USER));
      string16 user_input;
      bool finish = false;
      bool result;
      switch (LOWORD(wparam)) {
        case IDOK:
          finish = true;
          result = true;
          if (owner->message_type_ == JAVASCRIPT_MESSAGE_TYPE_PROMPT) {
            int length =
                GetWindowTextLength(GetDlgItem(dialog, IDC_PROMPTEDIT)) + 1;
            GetDlgItemText(dialog, IDC_PROMPTEDIT,
                           WriteInto(&user_input, length), length);
          }
          break;
        case IDCANCEL:
          finish = true;
          result = false;
          break;
      }
      if (finish) {
        owner->dialog_win_ = 0;
        owner->callback_.Run(result, user_input);
        DestroyWindow(dialog);
        owner->manager_->DialogClosed(owner);
      }
      break;
    }
    default:
      return DefWindowProc(dialog, message, wparam, lparam);
  }
  return 0;
}

JavaScriptDialog::JavaScriptDialog(
    ExoBrowserJavaScriptDialogManager* manager,
    gfx::NativeWindow parent_window,
    JavaScriptMessageType message_type,
    const string16& message_text,
    const string16& default_prompt_text,
    const JavaScriptDialogManager::DialogClosedCallback& callback)
    : manager_(manager),
      callback_(callback),
      message_text_(message_text),
      default_prompt_text_(default_prompt_text),
      message_type_(message_type) 
{
  int dialog_type;
  if (message_type == JAVASCRIPT_MESSAGE_TYPE_ALERT)
    dialog_type = IDD_ALERT;
  else if (message_type == JAVASCRIPT_MESSAGE_TYPE_CONFIRM)
    dialog_type = IDD_CONFIRM;
  else // JAVASCRIPT_MESSAGE_TYPE_PROMPT
    dialog_type = IDD_PROMPT;

  dialog_win_ = CreateDialogParam(GetModuleHandle(0),
                                  MAKEINTRESOURCE(dialog_type), 0, DialogProc,
                                  reinterpret_cast<LPARAM>(this));
  ShowWindow(dialog_win_, SW_SHOWNORMAL);
}

JavaScriptDialog::~JavaScriptDialog() 
{
  Cancel();
}

void 
JavaScriptDialog::Cancel() 
{
  if (dialog_win_)
    DestroyWindow(dialog_win_);
}

} // namespace exo_browser
