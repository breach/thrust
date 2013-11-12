// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/dialog/javascript_dialog_manager.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_util.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/browser/dialog/javascript_dialog.h"

using namespace content;

namespace exo_browser {

ExoBrowserJavaScriptDialogManager::ExoBrowserJavaScriptDialogManager() 
{
}

ExoBrowserJavaScriptDialogManager::~ExoBrowserJavaScriptDialogManager() 
{
}

void 
ExoBrowserJavaScriptDialogManager::RunJavaScriptDialog(
    WebContents* web_contents,
    const GURL& origin_url,
    const std::string& accept_lang,
    JavaScriptMessageType javascript_message_type,
    const string16& message_text,
    const string16& default_prompt_text,
    const DialogClosedCallback& callback,
    bool* did_suppress_message) 
{
  if (!dialog_request_callback_.is_null()) {
    dialog_request_callback_.Run();
    callback.Run(true, string16());
    dialog_request_callback_.Reset();
    return;
  }

#if defined(OS_MACOSX) || defined(OS_WIN) || defined(TOOLKIT_GTK)
  *did_suppress_message = false;

  if (dialog_) {
    /* One dialog at a time, please. */
    *did_suppress_message = true;
    return;
  }

  string16 new_message_text = net::FormatUrl(origin_url, accept_lang) +
                              ASCIIToUTF16("\n\n") +
                              message_text;
  gfx::NativeWindow parent_window =
      web_contents->GetView()->GetTopLevelNativeWindow();

  dialog_.reset(new JavaScriptDialog(this,
                                     parent_window,
                                     javascript_message_type,
                                     new_message_text,
                                     default_prompt_text,
                                     callback));
#else
  /* TODO(spolu): implement JavaScriptDialog for other platforms, */
  /* and then drop this #if                                                 */
  *did_suppress_message = true;
  return;
#endif
}

void 
ExoBrowserJavaScriptDialogManager::RunBeforeUnloadDialog(
    WebContents* web_contents,
    const string16& message_text,
    bool is_reload,
    const DialogClosedCallback& callback) 
{
  if (!dialog_request_callback_.is_null()) {
    dialog_request_callback_.Run();
    callback.Run(true, string16());
    dialog_request_callback_.Reset();
    return;
  }

#if defined(OS_MACOSX) || defined(OS_WIN) || defined(TOOLKIT_GTK)
  if (dialog_) {
    // Seriously!?
    callback.Run(true, string16());
    return;
  }

  string16 new_message_text =
      message_text +
      ASCIIToUTF16("\n\nIs it OK to leave/reload this page?");

  gfx::NativeWindow parent_window =
      web_contents->GetView()->GetTopLevelNativeWindow();

  dialog_.reset(new JavaScriptDialog(this,
                                     parent_window,
                                     JAVASCRIPT_MESSAGE_TYPE_CONFIRM,
                                     new_message_text,
                                     /* default prompt_text */
                                     string16(), 
                                     callback));
#else
  /* TODO(spolu): implement JavaScriptDialog for other platforms, */
  /* and then drop this #if                                       */
  callback.Run(true, string16());
  return;
#endif
}


void 
ExoBrowserJavaScriptDialogManager::CancelActiveAndPendingDialogs(
    WebContents* web_contents) 
{
#if defined(OS_MACOSX) || defined(OS_WIN) || defined(TOOLKIT_GTK)
  if (dialog_) {
    dialog_->Cancel();
    dialog_.reset();
  }
#else
  /* TODO(spolu): implement JavaScriptDialog for other platforms, */
  /* and then drop this #if                                       */
#endif
}

void 
ExoBrowserJavaScriptDialogManager::WebContentsDestroyed(
    WebContents* web_contents) 
{
}

void 
ExoBrowserJavaScriptDialogManager::DialogClosed(
    JavaScriptDialog* dialog) 
{
#if defined(OS_MACOSX) || defined(OS_WIN) || defined(TOOLKIT_GTK)
  DCHECK_EQ(dialog, dialog_.get());
  dialog_.reset();
#else
  /* TODO(spolu): implement JavaScriptDialog for other platforms, */
  /* and then drop this #if                                       */
#endif
}

} // namespace exo_browser
