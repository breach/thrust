// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/exo_frame.h"

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

} // namespace exo_browser

