// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.


#include "exo_browser/src/browser/exo_browser.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "base/file_util.h"
#include "base/threading/thread_restrictions.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia_rep.h"
#include "ui/gfx/codec/png_codec.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "exo_browser/src/browser/exo_frame.h"

using namespace content;

namespace exo_browser {

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
    int height,
    const std::string& icon_path)
{
  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(window_, "ExoBrowser");

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

  g_signal_connect(G_OBJECT(window_), "destroy",
                   G_CALLBACK(OnWindowDestroyedThunk), this);

  gtk_container_add(GTK_CONTAINER(window_), hbox_);
  gtk_window_resize(window_, width, height);

  /* Set the icon for the window before it gets displayed. We exceptionally */
  /* allow IO on this thread (UI) to load the PNG image.                    */
  base::ThreadRestrictions::SetIOAllowed(true);
  gfx::Image icon;
  base::FilePath p = base::FilePath::FromUTF8Unsafe(icon_path);
  // Read the file from disk.
  std::string file_contents;
  if(!p.empty() && base::ReadFileToString(p, &file_contents)) {
    // Decode the bitmap using WebKit's image decoder.
    const unsigned char* data =
      reinterpret_cast<const unsigned char*>(file_contents.data());
    scoped_ptr<SkBitmap> decoded(new SkBitmap());
    gfx::PNGCodec::Decode(data, file_contents.length(), decoded.get());
    if(!decoded->empty()) {
      icon = gfx::Image::CreateFrom1xBitmap(*decoded.release());
      gtk_window_set_icon(GTK_WINDOW(window_), icon.ToGdkPixbuf());
    }
  }
  base::ThreadRestrictions::SetIOAllowed(false);

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
  /* Nothing to Do */
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

void
ExoBrowser::PlatformMaximize()
{
  gtk_window_maximize(window_);
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
  LOG(INFO) << "WINDOW DESTROYED";
  /* We call Kill which will dispatch an event to the API as we don't expect */
  /* any other behavior here. If the ExoBrowser has not been already killed. */
  if(!is_killed_)
    Kill();
  return FALSE;  // Don't stop this message.
}

} // namespace exo_browser
