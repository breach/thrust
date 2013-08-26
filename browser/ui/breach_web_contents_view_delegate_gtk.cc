// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/ui/breach_web_contents_view_delegate.h"

#include "base/command_line.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/context_menu_params.h"
#include "content/shell/common/shell_switches.h"
#include "content/shell/shell.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_browser_main_parts.h"
#include "content/shell/shell_content_browser_client.h"
#include "content/shell/shell_devtools_frontend.h"
#include "content/shell/shell_web_contents_view_delegate_creator.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "ui/base/gtk/focus_store_gtk.h"
#include "ui/base/gtk/gtk_floating_container.h"

using namespace content;

using WebKit::WebContextMenuData;

namespace breach {

BreachWebContentsViewDelegate::BreachWebContentsViewDelegate(
    WebContents* web_contents)
    : web_contents_(web_contents),
      floating_(gtk_floating_container_new()) 
{
}

BreachWebContentsViewDelegate::~BreachWebContentsViewDelegate() 
{
  floating_.Destroy();
}

void 
BreachWebContentsViewDelegate::ShowContextMenu(
    const ContextMenuParams& params) 
{
  GtkWidget* menu = gtk_menu_new();

  params_ = params;
  bool has_link = !params_.unfiltered_link_url.is_empty();
  bool has_selection = !params_.selection_text.empty();

  if (params_.media_type == WebContextMenuData::MediaTypeNone &&
      !has_link &&
      !has_selection &&
      !params_.is_editable) {
    GtkWidget* back_menu = gtk_menu_item_new_with_label("Back");
    gtk_menu_append(GTK_MENU(menu), back_menu);
    g_signal_connect(back_menu,
                     "activate",
                     G_CALLBACK(OnBackMenuActivatedThunk),
                     this);
    gtk_widget_set_sensitive(back_menu,
                             web_contents_->GetController().CanGoBack());

    GtkWidget* forward_menu = gtk_menu_item_new_with_label("Forward");
    gtk_menu_append(GTK_MENU(menu), forward_menu);
    g_signal_connect(forward_menu,
                     "activate",
                     G_CALLBACK(OnForwardMenuActivatedThunk),
                     this);
    gtk_widget_set_sensitive(forward_menu,
                             web_contents_->GetController().CanGoForward());

    GtkWidget* reload_menu = gtk_menu_item_new_with_label("Reload");
    gtk_menu_append(GTK_MENU(menu), reload_menu);
    g_signal_connect(reload_menu,
                     "activate",
                     G_CALLBACK(OnReloadMenuActivatedThunk),
                     this);

    GtkWidget* navigate_separator = gtk_separator_menu_item_new();
    gtk_menu_append(GTK_MENU(menu), navigate_separator);
  }

  if (has_link) {
    GtkWidget* open_menu = gtk_menu_item_new_with_label("Open Link in New Tab");
    gtk_menu_append(GTK_MENU(menu), open_menu);
    g_signal_connect(open_menu,
                     "activate",
                     G_CALLBACK(OnOpenURLMenuActivatedThunk),
                     this);

    GtkWidget* link_separator = gtk_separator_menu_item_new();
    gtk_menu_append(GTK_MENU(menu), link_separator);
  }

  if (params_.is_editable) {
    GtkWidget* cut_menu = gtk_menu_item_new_with_label("Cut");
    gtk_menu_append(GTK_MENU(menu), cut_menu);
    g_signal_connect(cut_menu,
                     "activate",
                     G_CALLBACK(OnCutMenuActivatedThunk),
                     this);
    gtk_widget_set_sensitive(
        cut_menu,
        params_.edit_flags & WebContextMenuData::CanCut);

    GtkWidget* copy_menu = gtk_menu_item_new_with_label("Copy");
    gtk_menu_append(GTK_MENU(menu), copy_menu);
    g_signal_connect(copy_menu,
                     "activate",
                     G_CALLBACK(OnCopyMenuActivatedThunk),
                     this);
    gtk_widget_set_sensitive(
        copy_menu,
        params_.edit_flags & WebContextMenuData::CanCopy);

    GtkWidget* paste_menu = gtk_menu_item_new_with_label("Paste");
    gtk_menu_append(GTK_MENU(menu), paste_menu);
    g_signal_connect(paste_menu,
                     "activate",
                     G_CALLBACK(OnPasteMenuActivatedThunk),
                     this);
    gtk_widget_set_sensitive(
        paste_menu,
        params_.edit_flags & WebContextMenuData::CanPaste);

    GtkWidget* delete_menu = gtk_menu_item_new_with_label("Delete");
    gtk_menu_append(GTK_MENU(menu), delete_menu);
    g_signal_connect(delete_menu,
                     "activate",
                     G_CALLBACK(OnDeleteMenuActivatedThunk),
                     this);
    gtk_widget_set_sensitive(
        delete_menu,
        params_.edit_flags & WebContextMenuData::CanDelete);

    GtkWidget* edit_separator = gtk_separator_menu_item_new();
    gtk_menu_append(GTK_MENU(menu), edit_separator);
  } 
  else if (has_selection) {
    GtkWidget* copy_menu = gtk_menu_item_new_with_label("Copy");
    gtk_menu_append(GTK_MENU(menu), copy_menu);
    g_signal_connect(copy_menu,
                     "activate",
                     G_CALLBACK(OnCopyMenuActivatedThunk),
                     this);

    GtkWidget* copy_separator = gtk_separator_menu_item_new();
    gtk_menu_append(GTK_MENU(menu), copy_separator);
  }

  GtkWidget* inspect_menu = gtk_menu_item_new_with_label("Inspect");
  gtk_menu_append(GTK_MENU(menu), inspect_menu);
  g_signal_connect(inspect_menu,
                   "activate",
                   G_CALLBACK(OnInspectMenuActivatedThunk),
                   this);

  gtk_widget_show_all(menu);

  gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, GDK_CURRENT_TIME);
}

WebDragDestDelegate* 
BreachWebContentsViewDelegate::GetDragDestDelegate() 
{
  return NULL;
}

void 
BreachWebContentsViewDelegate::Initialize(
    GtkWidget* expanded_container,
    ui::FocusStoreGtk* focus_store) 
{
  expanded_container_ = expanded_container;

  gtk_container_add(GTK_CONTAINER(floating_.get()), expanded_container_);
  gtk_widget_show(floating_.get());
}

gfx::NativeView 
BreachWebContentsViewDelegate::GetNativeView() const 
{
  return floating_.get();
}

void 
BreachWebContentsViewDelegate::Focus() 
{
  GtkWidget* widget = web_contents_->GetView()->GetContentNativeView();
  if(widget)
    gtk_widget_grab_focus(widget);
}

gboolean 
BreachWebContentsViewDelegate::OnNativeViewFocusEvent(
    GtkWidget* widget,
    GtkDirectionType type,
    gboolean* return_value) {
  return false;
}

void 
BreachWebContentsViewDelegate::OnBackMenuActivated(
    GtkWidget* widget) 
{
  web_contents_->GetController().GoToOffset(-1);
  web_contents_->GetView()->Focus();
}

void 
BreachWebContentsViewDelegate::OnForwardMenuActivated(
    GtkWidget* widget) 
{
  web_contents_->GetController().GoToOffset(1);
  web_contents_->GetView()->Focus();
}

void 
BreachWebContentsViewDelegate::OnReloadMenuActivated(
    GtkWidget* widget) 
{
  web_contents_->GetController().Reload(false);
  web_contents_->GetView()->Focus();
}

void 
BreachWebContentsViewDelegate::OnOpenURLMenuActivated(
    GtkWidget* widget) 
{
  /* TODO(spolu): Handle */
  /*
  ShellBrowserContext* browser_context =
      ShellContentBrowserClient::Get()->browser_context();
  Shell::CreateNewWindow(browser_context,
                         params_.link_url,
                         NULL,
                         MSG_ROUTING_NONE,
                         gfx::Size());
  */
}

void 
BreachWebContentsViewDelegate::OnCutMenuActivated(
    GtkWidget* widget) 
{
  web_contents_->GetRenderViewHost()->Cut();
}

void 
BreachWebContentsViewDelegate::OnCopyMenuActivated(
    GtkWidget* widget) 
{
  web_contents_->GetRenderViewHost()->Copy();
}

void 
BreachWebContentsViewDelegate::OnPasteMenuActivated(
    GtkWidget* widget) 
{
  web_contents_->GetRenderViewHost()->Paste();
}

void 
BreachWebContentsViewDelegate::OnDeleteMenuActivated(
    GtkWidget* widget) 
{
  web_contents_->GetRenderViewHost()->Delete();
}

void 
BreachWebContentsViewDelegate::OnInspectMenuActivated(
    GtkWidget* widget) 
{
  /* TODO(spolu): Handle when DevTools are implemented */
  //BreachDevToolsFrontend::Show(web_contents_);
}

}  // namespace content
