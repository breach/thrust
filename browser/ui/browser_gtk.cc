// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/ui/browser.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/renderer_preferences.h"
#include "breach/browser/breach_browser_context.h"
#include "breach/browser/breach_content_browser_client.h"

using namespace content;

namespace breach {

namespace {

// Callback for Debug > Show web inspector... menu item.
gboolean 
ShowWebInspectorActivated(
    GtkWidget* widget, 
    Browser* browser) 
{
  browser->ShowDevTools();
  return FALSE;  // Don't stop this message.
}

GtkWidget* 
AddMenuEntry(
    GtkWidget* menu_widget, 
    const char* text,
    GCallback callback, 
    Browser* browser) 
{
  GtkWidget* entry = gtk_menu_item_new_with_label(text);
  g_signal_connect(entry, "activate", callback, browser);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_widget), entry);
  return entry;
}

GtkWidget* 
CreateMenu(
    GtkWidget* menu_bar, 
    const char* text) 
{
  GtkWidget* menu_widget = gtk_menu_new();
  GtkWidget* menu_header = gtk_menu_item_new_with_label(text);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_header), menu_widget);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_header);
  return menu_widget;
}

GtkWidget* 
CreateMenuBar(
    Browser* browser) 
{
  GtkWidget* menu_bar = gtk_menu_bar_new();
  GtkWidget* debug_menu = CreateMenu(menu_bar, "Debug");
  AddMenuEntry(debug_menu, "Show web inspector...",
               G_CALLBACK(ShowWebInspectorActivated), browser);
  return menu_bar;
}

}  // namespace

void 
Browser::PlatformInitialize(
    const gfx::Size& default_window_size) 
{
}

void 
Browser::PlatformCleanUp() 
{
  // Nothing to clean up; GTK will clean up the widgets shortly after.
}

void 
Browser::PlatformEnableUIControl(
    UIControl control,
    bool is_enabled) 
{
  if (headless_)
    return;

  GtkToolItem* item = NULL;
  switch (control) {
    case BACK_BUTTON:
      item = back_button_;
      break;
    case FORWARD_BUTTON:
      item = forward_button_;
      break;
    case STOP_BUTTON:
      item = stop_button_;
      break;
    default:
      NOTREACHED() << "Unknown UI control";
      return;
  }
  gtk_widget_set_sensitive(GTK_WIDGET(item), is_enabled);
}

void 
Browser::PlatformSetAddressBarURL(
    const GURL& url) 
{
  if (headless_)
    return;

  gtk_entry_set_text(GTK_ENTRY(url_edit_view_), url.spec().c_str());
}

void 
Browser::PlatformSetIsLoading(
    bool loading) 
{
  if (headless_)
    return;

  if (loading)
    gtk_spinner_start(GTK_SPINNER(spinner_));
  else
    gtk_spinner_stop(GTK_SPINNER(spinner_));
}

void 
Browser::PlatformCreateWindow(
    int width, 
    int height) 
{
  ui_elements_height_ = 0;
  if (headless_) {
    SizeTo(width, height);
    return;
  }

  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(window_, "Breach");
  g_signal_connect(G_OBJECT(window_), "destroy",
                   G_CALLBACK(OnWindowDestroyedThunk), this);

  vbox_ = gtk_vbox_new(FALSE, 0);

  // Create the menu bar.
  GtkWidget* menu_bar = CreateMenuBar(this);
  gtk_box_pack_start(GTK_BOX(vbox_), menu_bar, FALSE, FALSE, 0);

  // Create the object that mediates accelerators.
  GtkAccelGroup* accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window_), accel_group);

  // Set global window handling accelerators:
  gtk_accel_group_connect(
      accel_group, GDK_w, GDK_CONTROL_MASK,
      GTK_ACCEL_VISIBLE,
      g_cclosure_new(G_CALLBACK(OnCloseWindowKeyPressedThunk),
                     this, NULL));

  gtk_accel_group_connect(
      accel_group, GDK_n, GDK_CONTROL_MASK,
      GTK_ACCEL_VISIBLE,
      g_cclosure_new(G_CALLBACK(OnNewWindowKeyPressedThunk),
                    this, NULL));

  gtk_accel_group_connect(
    accel_group, GDK_F5, (GdkModifierType)0,
      GTK_ACCEL_VISIBLE,
      g_cclosure_new(G_CALLBACK(OnReloadKeyPressedThunk),
                    this, NULL));

  GtkWidget* toolbar = gtk_toolbar_new();
  // Turn off the labels on the toolbar buttons.
  gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

  back_button_ = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
  g_signal_connect(back_button_, "clicked",
                   G_CALLBACK(&OnBackButtonClickedThunk), this);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), back_button_, -1 /* append */);
  gtk_widget_add_accelerator(GTK_WIDGET(back_button_), "clicked", accel_group,
                             GDK_Left, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

  forward_button_ = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
  g_signal_connect(forward_button_, "clicked",
                   G_CALLBACK(&OnForwardButtonClickedThunk), this);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), forward_button_, -1 /* append */);
  gtk_widget_add_accelerator(GTK_WIDGET(forward_button_), "clicked",
                             accel_group,
                             GDK_Right, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

  reload_button_ = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
  g_signal_connect(reload_button_, "clicked",
                   G_CALLBACK(&OnReloadButtonClickedThunk), this);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), reload_button_, -1 /* append */);
  gtk_widget_add_accelerator(GTK_WIDGET(reload_button_), "clicked",
                             accel_group,
                             GDK_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  stop_button_ = gtk_tool_button_new_from_stock(GTK_STOCK_STOP);
  g_signal_connect(stop_button_, "clicked",
                   G_CALLBACK(&OnStopButtonClickedThunk), this);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), stop_button_, -1 /* append */);

  url_edit_view_ = gtk_entry_new();
  g_signal_connect(G_OBJECT(url_edit_view_), "activate",
                   G_CALLBACK(&OnURLEntryActivateThunk), this);

  gtk_accel_group_connect(
      accel_group, GDK_l, GDK_CONTROL_MASK,
      GTK_ACCEL_VISIBLE,
      g_cclosure_new(G_CALLBACK(OnHighlightURLViewThunk),
                     this, NULL));

  GtkToolItem* tool_item = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(tool_item), url_edit_view_);
  gtk_tool_item_set_expand(tool_item, TRUE);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), tool_item, -1 /* append */);

  // Center a 20x20 spinner in a 26x24 area.
  GtkWidget* spinner_alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
  gtk_alignment_set_padding(GTK_ALIGNMENT(spinner_alignment), 2, 2, 4, 4);
  spinner_ = gtk_spinner_new();
  gtk_widget_set_size_request(spinner_, 20, 20);
  gtk_container_add(GTK_CONTAINER(spinner_alignment), spinner_);

  spinner_item_ = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(spinner_item_), spinner_alignment);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), spinner_item_, -1 /* append */);

  gtk_box_pack_start(GTK_BOX(vbox_), toolbar, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(window_), vbox_);

  // Trigger layout of the UI elements, so that we can measure their
  // heights. The width and height passed to this method are meant for the web
  // contents view, not the top-level window. Since Gtk only seems to provide a
  // suitable resizing function for top-level windows, we need to know how to
  // convert from web contents view size to top-level window size.
  gtk_widget_show_all(GTK_WIDGET(vbox_));

  // Measure the heights of the UI elements, now that they have been laid out.
  GtkRequisition elm_size;
  gtk_widget_size_request(menu_bar, &elm_size);
  ui_elements_height_ += elm_size.height;
  gtk_widget_size_request(toolbar, &elm_size);
  ui_elements_height_ += elm_size.height;

  // We're ready to set an initial window size.
  SizeTo(width, height);

  // Finally, show the window.
  gtk_widget_show_all(GTK_WIDGET(window_));
}

void 
Browser::PlatformSetContents() 
{
  if (headless_)
    return;

  WebContentsView* content_view = web_contents_->GetView();
  gtk_container_add(GTK_CONTAINER(vbox_), content_view->GetNativeView());
}

void 
Browser::SizeTo(
    int width, 
    int height) 
{
  content_width_ = width;
  content_height_ = height;

  // Prefer setting the top level window's size (if we have one), rather than
  // setting the inner widget's minimum size (so that the user can shrink the
  // window if she wants).
  if (window_) {
    gtk_window_resize(window_, width, height + ui_elements_height_);
  } else if (web_contents_) {
    gtk_widget_set_size_request(web_contents_->GetView()->GetNativeView(),
                                width, height);
  }
}

void 
Browser::PlatformResizeSubViews() 
{
  SizeTo(content_width_, content_height_);
}

void 
Browser::Close() 
{
  if (headless_) {
    delete this;
    return;
  }

  gtk_widget_destroy(GTK_WIDGET(window_));
}

void 
Browser::OnBackButtonClicked(
    GtkWidget* widget) 
{
  GoBackOrForward(-1);
}

void 
Browser::OnForwardButtonClicked(
    GtkWidget* widget) 
{
  GoBackOrForward(1);
}

void 
Browser::OnReloadButtonClicked(
    GtkWidget* widget) 
{
  Reload();
}

void 
Browser::OnStopButtonClicked(
    GtkWidget* widget) 
{
  Stop();
}

void 
Browser::OnURLEntryActivate(
    GtkWidget* entry) 
{
  const gchar* str = gtk_entry_get_text(GTK_ENTRY(entry));
  GURL url(str);
  if (!url.has_scheme())
    url = GURL(std::string("http://") + std::string(str));
  LoadURL(GURL(url));
}

// Callback for when the main window is destroyed.
gboolean 
Browser::OnWindowDestroyed(
    GtkWidget* window) 
{
  delete this;
  return FALSE;  // Don't stop this message.
}

gboolean 
Browser::OnCloseWindowKeyPressed(
    GtkAccelGroup* accel_group,
    GObject* acceleratable,
    guint keyval,
    GdkModifierType modifier) 
{
  gtk_widget_destroy(GTK_WIDGET(window_));
  return TRUE;
}

gboolean 
Browser::OnNewWindowKeyPressed(
    GtkAccelGroup* accel_group,
    GObject* acceleratable,
    guint keyval,
    GdkModifierType modifier) 
{
  BreachBrowserContext* browser_context =
      BreachContentBrowserClient::Get()->browser_context();
  Browser::CreateNewWindow(browser_context,
                           GURL(),
                           NULL,
                           MSG_ROUTING_NONE,
                           gfx::Size());
  return TRUE;
}

gboolean 
Browser::OnHighlightURLView(
    GtkAccelGroup* accel_group,
    GObject* acceleratable,
    guint keyval,
    GdkModifierType modifier) 
{
  gtk_widget_grab_focus(GTK_WIDGET(url_edit_view_));
  return TRUE;
}

gboolean 
Browser::OnReloadKeyPressed(
    GtkAccelGroup* accel_group,
    GObject* acceleratable,
    guint keyval,
    GdkModifierType modifier) 
{
  Reload();
  return TRUE;
}

void 
Browser::PlatformSetTitle(
    const string16& title) 
{
  if (headless_)
    return;

  std::string title_utf8 = UTF16ToUTF8(title);
  gtk_window_set_title(GTK_WINDOW(window_), title_utf8.c_str());
}

}  // namespace breach
