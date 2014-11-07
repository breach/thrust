// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_WEB_VIEW_JAVASCRIPT_DIALOG_MANAGER_H_
#define THRUST_SHELL_BROWSER_WEB_VIEW_JAVASCRIPT_DIALOG_MANAGER_H_

#include "base/callback_forward.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/javascript_dialog_manager.h"

namespace thrust_shell {

class WebViewGuest;

class WebViewGuestJavaScriptDialogManager : 
    public content::JavaScriptDialogManager {
 public:
  WebViewGuestJavaScriptDialogManager(WebViewGuest* guest);
  virtual ~WebViewGuestJavaScriptDialogManager();

  // JavaScriptDialogManager overrides
  virtual void RunJavaScriptDialog(
      content::WebContents* web_contents,
      const GURL& origin_url,
      const std::string& accept_lang,
      content::JavaScriptMessageType javascript_message_type,
      const base::string16& message_text,
      const base::string16& default_prompt_text,
      const DialogClosedCallback& callback,
      bool* did_suppress_message) OVERRIDE;

  virtual void RunBeforeUnloadDialog(
      content::WebContents* web_contents,
      const base::string16& message_text,
      bool is_reload,
      const DialogClosedCallback& callback) OVERRIDE;

  virtual void CancelActiveAndPendingDialogs(
      content::WebContents* web_contents) OVERRIDE {}

  virtual void WebContentsDestroyed(
      content::WebContents* web_contents) OVERRIDE;

  void JavaScriptDialogClosed(bool success,
                              const std::string& response);

 private:
  WebViewGuest*                       guest_;
  //const content::DialogClosedCallback dialog_callback_;
  base::Callback<void(bool,
                      const base::string16&)>  dialog_callback_;

  DISALLOW_COPY_AND_ASSIGN(WebViewGuestJavaScriptDialogManager);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_WEB_VIEW_JAVASCRIPT_DIALOG_MANAGER_H_
