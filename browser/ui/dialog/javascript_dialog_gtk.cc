// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/ui/dialog/javascript_dialog.h"

#include <gtk/gtk.h>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "breach/browser/ui/browser.h"
#include "breach/browser/ui/dialog/breach_javascript_dialog_manager.h"

using namespace content;

namespace {

const char kPromptTextId[] = "breach_prompt_text";

// If there's a text entry in the dialog, get the text from the first one and
// return it.
string16 GetPromptText(GtkDialog* dialog) {
  GtkWidget* widget = static_cast<GtkWidget*>(
      g_object_get_data(G_OBJECT(dialog), kPromptTextId));
  if (widget)
    return UTF8ToUTF16(gtk_entry_get_text(GTK_ENTRY(widget)));
  return string16();
}

}  // namespace


namespace breach {

JavaScriptDialog::JavaScriptDialog(
    BreachJavaScriptDialogManager* manager,
    gfx::NativeWindow parent_window,
    JavaScriptMessageType message_type,
    const string16& message_text,
    const string16& default_prompt_text,
    const JavaScriptDialogManager::DialogClosedCallback& callback)
    : manager_(manager),
      callback_(callback),
      parent_window_(parent_window) 
{
  GtkButtonsType buttons = GTK_BUTTONS_NONE;
  GtkMessageType gtk_message_type = GTK_MESSAGE_OTHER;

  switch (message_type) {
    case content::JAVASCRIPT_MESSAGE_TYPE_ALERT:
      buttons = GTK_BUTTONS_NONE;
      gtk_message_type = GTK_MESSAGE_WARNING;
      break;

    case content::JAVASCRIPT_MESSAGE_TYPE_CONFIRM:
      buttons = GTK_BUTTONS_CANCEL;
      gtk_message_type = GTK_MESSAGE_QUESTION;
      break;

    case content::JAVASCRIPT_MESSAGE_TYPE_PROMPT:
      buttons = GTK_BUTTONS_CANCEL;
      gtk_message_type = GTK_MESSAGE_QUESTION;
      break;

    default:
      NOTREACHED();
  }

  gtk_dialog_ = gtk_message_dialog_new(parent_window_,
                                       GTK_DIALOG_MODAL,
                                       gtk_message_type,
                                       buttons,
                                       "%s",
                                       UTF16ToUTF8(message_text).c_str());
  g_signal_connect(gtk_dialog_,
                   "delete-event",
                   G_CALLBACK(gtk_widget_hide_on_delete),
                   NULL);
  gtk_window_set_title(GTK_WINDOW(gtk_dialog_), "JavaScript");

  GtkWidget* ok_button = gtk_dialog_add_button(GTK_DIALOG(gtk_dialog_),
                                               GTK_STOCK_OK,
                                               GTK_RESPONSE_OK);

  if (message_type != content::JAVASCRIPT_MESSAGE_TYPE_PROMPT)
    gtk_widget_grab_focus(ok_button);

  if (message_type == content::JAVASCRIPT_MESSAGE_TYPE_PROMPT) {
    GtkWidget* content_area =
        gtk_dialog_get_content_area(GTK_DIALOG(gtk_dialog_));
    GtkWidget* text_box = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(text_box),
                       UTF16ToUTF8(default_prompt_text).c_str());
    gtk_box_pack_start(GTK_BOX(content_area), text_box, TRUE, TRUE, 0);
    g_object_set_data(G_OBJECT(gtk_dialog_), kPromptTextId, text_box);
    gtk_entry_set_activates_default(GTK_ENTRY(text_box), TRUE);
  }

  gtk_dialog_set_default_response(GTK_DIALOG(gtk_dialog_), GTK_RESPONSE_OK);
  g_signal_connect(gtk_dialog_, "response", G_CALLBACK(OnResponseThunk), this);
  gtk_widget_show_all(GTK_WIDGET(gtk_dialog_));
}

JavaScriptDialog::~JavaScriptDialog() 
{
}

void 
JavaScriptDialog::Cancel() 
{
}

void 
JavaScriptDialog::OnResponse(
    GtkWidget* dialog, 
    int response_id) 
{
  switch (response_id) {
    case GTK_RESPONSE_OK:
      callback_.Run(true, GetPromptText(GTK_DIALOG(dialog)));
      break;
    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_DELETE_EVENT:
      callback_.Run(false, string16());
      break;
    default:
      NOTREACHED();
  }

  gtk_widget_destroy(dialog);

  manager_->DialogClosed(this);
}

}  // namespace breach
