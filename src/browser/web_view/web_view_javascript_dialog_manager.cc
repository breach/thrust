// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/web_view/web_view_javascript_dialog_manager.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_util.h"
#include "content/public/browser/web_contents.h"

#include "src/common/switches.h"
#include "src/browser/web_view/web_view_guest.h"
#include "src/browser/thrust_window.h"

using namespace content;

namespace {

std::string JavaScriptMessageTypeToString(
  JavaScriptMessageType javascript_message_type) {
  switch (javascript_message_type) {
    case JAVASCRIPT_MESSAGE_TYPE_ALERT:
      return "alert";
    case JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
      return "confirm";
    case JAVASCRIPT_MESSAGE_TYPE_PROMPT:
      return "prompt";
    default:
      NOTREACHED() << "Unknown Javascript Message Type";
      return "ignore";
  }
}

} // namespace

namespace thrust_shell {

WebViewGuestJavaScriptDialogManager::WebViewGuestJavaScriptDialogManager(
    WebViewGuest* guest)
  : guest_(guest)
{
}

WebViewGuestJavaScriptDialogManager::~WebViewGuestJavaScriptDialogManager() 
{
}

void 
WebViewGuestJavaScriptDialogManager::RunJavaScriptDialog(
    WebContents* web_contents,
    const GURL& origin_url,
    const std::string& accept_lang,
    JavaScriptMessageType javascript_message_type,
    const base::string16& message_text,
    const base::string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) 
{
  if(!guest_) {
    *did_suppress_message = true;
    return;
  }

  dialog_callback_ = callback;

  base::DictionaryValue event;
  event.SetString("origin_url", origin_url.spec());
  event.SetString("accept_lang", accept_lang);
  event.SetString("message_type", 
                  JavaScriptMessageTypeToString(javascript_message_type));
  event.SetString("message_text", message_text);
  event.SetString("default_prompt_text", default_prompt_text);

  guest_->GetThrustWindow()->WebViewEmit(
      guest_->guest_instance_id_,
      "dialog",
      event);
}

void 
WebViewGuestJavaScriptDialogManager::RunBeforeUnloadDialog(
    WebContents* web_contents,
    const base::string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) 
{
  callback.Run(true, base::string16());
}

void 
WebViewGuestJavaScriptDialogManager::WebContentsDestroyed(
    content::WebContents* web_contents)
{
  guest_ = NULL;
}

void 
WebViewGuestJavaScriptDialogManager::JavaScriptDialogClosed(
    bool success,
    const std::string& response)
{
  if(!dialog_callback_.is_null()) {
    dialog_callback_.Run(success, base::UTF8ToUTF16(response));
    dialog_callback_.Reset();
  }
}

} // namespace thrust_shell
