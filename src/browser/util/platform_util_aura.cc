// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/util/platform_util.h"

#include "base/logging.h"
#include "ui/aura/window.h"

#if defined(USE_ASH)
#include "ash/wm/window_util.h"
#endif

namespace platform_util {

gfx::NativeWindow GetTopLevel(gfx::NativeView view) {
  return view->GetToplevelWindow();
}

gfx::NativeView GetParent(gfx::NativeView view) {
  return view->parent();
}

bool IsWindowActive(gfx::NativeWindow window) {
#if defined(USE_ASH)
  return ash::wm::IsActiveWindow(window);
#else
  NOTIMPLEMENTED();
  return false;
#endif
}

void ActivateWindow(gfx::NativeWindow window) {
#if defined(USE_ASH)
  ash::wm::ActivateWindow(window);
#else
  NOTIMPLEMENTED();
#endif
}

bool IsVisible(gfx::NativeView view) {
  return view->IsVisible();
}

}  // namespace platform_util
