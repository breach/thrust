// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.


#include "breach/browser/ui/exo_browser.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "breach/browser/ui/exo_frame.h"

using namespace content;

namespace breach {

void 
ExoBrowser::PlatformInitialize(
    const gfx::Size& default_window_size) 
{
}

void 
ExoBrowser::PlatformCleanUp() 
{
  /* Nothing to clean up; GTK will clean up the widgets shortly after. */
}

void 
ExoBrowser::PlatformCreateWindow(
    int width,
    int height)
{
  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(window_, "Breach");

  hbox_ = gtk_hbox_new(FALSE, 0);
  vbox_ = gtk_vbox_new(FALSE, 0);

  control_left_box_ = gtk_event_box_new();
  control_right_box_ = gtk_event_box_new();
  control_top_box_ = gtk_event_box_new();
  control_bottom_box_ = gtk_event_box_new();

  gtk_widget_set_size_request(control_left_box_, 0, 0);
  gtk_widget_set_size_request(control_right_box_, 0, 0);
  gtk_widget_set_size_request(control_top_box_, 0, 0);
  gtk_widget_set_size_request(control_bottom_box_, 0, 0);
  
  pages_box_ = gtk_event_box_new();
  visible_page_ = NULL;

  // Create the menu bar.
  gtk_box_pack_start(GTK_BOX(hbox_), control_left_box_, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_), vbox_, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_), control_right_box_, FALSE, FALSE, 0);

  gtk_box_pack_start(GTK_BOX(vbox_), control_top_box_, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_), pages_box_, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_), control_bottom_box_, FALSE, FALSE, 0);

  // Create the object that mediates accelerators.
  GtkAccelGroup* accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window_), accel_group);

  g_signal_connect(G_OBJECT(window_), "destroy",
                   G_CALLBACK(OnWindowDestroyedThunk), this);

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

  gtk_container_add(GTK_CONTAINER(window_), hbox_);
  gtk_window_resize(window_, width, height);
  /* TODO(spolu): move to API */
  gtk_window_maximize(window_);

  // Finally, show the window.
  gtk_widget_show_all(GTK_WIDGET(window_));
}

void 
ExoBrowser::PlatformKill() 
{
  gtk_widget_destroy(GTK_WIDGET(window_));
}

void 
ExoBrowser::PlatformSetTitle(
    const std::string& title) 
{
  gtk_window_set_title(GTK_WINDOW(window_), title.c_str());
}



void 
ExoBrowser::PlatformAddPage(
    ExoFrame *frame)
{
  /* Nothing to Do? */
}


void 
ExoBrowser::PlatformRemovePage(
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  if(visible_page_ == content_view->GetNativeView()) {
    gtk_container_remove(GTK_CONTAINER(pages_box_), visible_page_);
    visible_page_ = NULL;
  }
}


void 
ExoBrowser::PlatformShowPage(
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  if(visible_page_ != content_view->GetNativeView()) {
    if(visible_page_ != NULL) {
      gtk_container_remove(GTK_CONTAINER(pages_box_), visible_page_);
    }
    visible_page_ = content_view->GetNativeView();
    gtk_container_add(GTK_CONTAINER(pages_box_), visible_page_);
  }
}


void 
ExoBrowser::PlatformSetControl(
    CONTROL_TYPE type, 
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  LOG(INFO) << "PlatformSetControl: " << type << " " << frame->name();

  switch(type) {
    case LEFT_CONTROL: 
      gtk_container_add(GTK_CONTAINER(control_left_box_),
                        content_view->GetNativeView());
      break;
    case RIGHT_CONTROL: 
      gtk_container_add(GTK_CONTAINER(control_right_box_),
                        content_view->GetNativeView());
      break;
    case TOP_CONTROL: 
      gtk_container_add(GTK_CONTAINER(control_top_box_),
                        content_view->GetNativeView());
      break;
    case BOTTOM_CONTROL: 
      gtk_container_add(GTK_CONTAINER(control_bottom_box_),
                        content_view->GetNativeView());
      break;
    default:
      /* Nothing to do */
      ;
  }
}


void 
ExoBrowser::PlatformSetControlDimension(
    CONTROL_TYPE type, 
    int size)
{
  LOG(INFO) << "PlatformSetControlDimension: " 
            << type << " " << size;

  switch(type) {
    case LEFT_CONTROL: 
      gtk_widget_set_size_request(control_left_box_, size, 0);
      break;
    case RIGHT_CONTROL: 
      gtk_widget_set_size_request(control_right_box_, size, 0);
      break;
    case TOP_CONTROL: 
      gtk_widget_set_size_request(control_top_box_, 0, size);
      break;
    case BOTTOM_CONTROL: 
      gtk_widget_set_size_request(control_bottom_box_, 0, size);
      break;
    default:
      /* Nothing to do */
      ;
  }
}


void 
ExoBrowser::PlatformUnsetControl(
    CONTROL_TYPE type, 
    ExoFrame *frame)
{
  WebContentsView* content_view = frame->web_contents_->GetView();
  LOG(INFO) << "PlatformUnsetControl: " << type << " " << frame->name();

  switch(type) {
    case LEFT_CONTROL: 
      gtk_container_remove(GTK_CONTAINER(control_left_box_),
                           content_view->GetNativeView());
      gtk_widget_set_size_request(control_left_box_, 0, 0);
      break;
    case RIGHT_CONTROL: 
      gtk_container_remove(GTK_CONTAINER(control_right_box_),
                           content_view->GetNativeView());
      gtk_widget_set_size_request(control_right_box_, 0, 0);
      break;
    case TOP_CONTROL: 
      gtk_container_remove(GTK_CONTAINER(control_top_box_),
                           content_view->GetNativeView());
      gtk_widget_set_size_request(control_top_box_, 0, 0);
      break;
    case BOTTOM_CONTROL: 
      gtk_container_remove(GTK_CONTAINER(control_bottom_box_),
                           content_view->GetNativeView());
      gtk_widget_set_size_request(control_bottom_box_, 0, 0);
      break;
    default:
      /* Nothing to do */
      ;
  }
}


void
ExoBrowser::PlatformFocus()
{
  gtk_window_present(window_);
}


gfx::Size
ExoBrowser::PlatformSize()
{
  int w,h;
  gtk_window_get_size(window_, &w, &h);
  return gfx::Size(w, h);
}

gfx::Point
ExoBrowser::PlatformPosition()
{
  int x,y;
  gtk_window_get_position(window_, &x, &y);
  return gfx::Point(x, y);
}

gboolean 
ExoBrowser::OnWindowDestroyed(
    GtkWidget* window) 
{
  /* We call Kill which will dispatch an event to the API as we don't expect */
  /* any other behavior here                                                 */
  Kill();
  return FALSE;  // Don't stop this message.
}

gboolean 
ExoBrowser::OnCloseWindowKeyPressed(
    GtkAccelGroup* accel_group,
    GObject* acceleratable,
    guint keyval,
    GdkModifierType modifier) 
{
  /* TODO(spolu): call back into API */
  LOG(INFO) << "OnCloseWindowKeyPressed";
  return TRUE;
}

gboolean 
ExoBrowser::OnNewWindowKeyPressed(
    GtkAccelGroup* accel_group,
    GObject* acceleratable,
    guint keyval,
    GdkModifierType modifier) 
{
  /* TODO(spolu): call back into API */
  LOG(INFO) << "OnNewWindowKeyPressed";
  return TRUE;
}

} // namespace breach
