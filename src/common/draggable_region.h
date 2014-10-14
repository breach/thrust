// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_COMMON_DRAGGABLE_REGION_H_
#define THRUST_SHELL_COMMON_DRAGGABLE_REGION_H_

#include "ui/gfx/rect.h"

namespace thrust_shell {

struct DraggableRegion {
  bool draggable;
  gfx::Rect bounds;

  DraggableRegion();
};

}  // namespace thrust_shell

#endif  // THRUST_SHELL_COMMON_DRAGGABLE_REGION_H_
