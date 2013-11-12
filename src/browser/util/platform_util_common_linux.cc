// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/util/platform_util.h"

#include <gtk/gtk.h>

namespace platform_util {

gfx::NativeWindow GetTopLevel(gfx::NativeView view) {
  // A detached widget won't have a toplevel window as an ancestor, so we can't
  // assume that the query for toplevel will return a window.
  GtkWidget* toplevel = gtk_widget_get_ancestor(view, GTK_TYPE_WINDOW);
  return GTK_IS_WINDOW(toplevel) ? GTK_WINDOW(toplevel) : NULL;
}

gfx::NativeView GetParent(gfx::NativeView view) {
  return gtk_widget_get_parent(view);
}

bool IsWindowActive(gfx::NativeWindow window) {
  return gtk_window_is_active(window);
}

void ActivateWindow(gfx::NativeWindow window) {
  gtk_window_present(window);
}

bool IsVisible(gfx::NativeView view) {
  return gtk_widget_get_visible(view);
}

}  // namespace platform_util
