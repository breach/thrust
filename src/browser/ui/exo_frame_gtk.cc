// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "exo_browser/src/browser/ui/exo_frame.h"

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"

using namespace content;

namespace exo_browser {

gfx::Size
ExoFrame::PlatformSize()
{
  WebContentsView* content_view = web_contents_->GetView();
  GtkRequisition size;
  gtk_widget_size_request(content_view->GetNativeView(), &size);
  return gfx::Size(size.width, size.height);
}

void
ExoFrame::PlatformFocus()
{
  WebContentsView* content_view = web_contents_->GetView();
  gtk_widget_set_can_focus(content_view->GetNativeView(), true);
  gtk_widget_grab_focus(content_view->GetNativeView());
}

} // namespace exo_browser

