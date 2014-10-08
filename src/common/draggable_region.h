// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_COMMON_DRAGGABLE_REGION_H_
#define EXO_SHELL_COMMON_DRAGGABLE_REGION_H_

#include "ui/gfx/rect.h"

namespace exo_shell {

struct DraggableRegion {
  bool draggable;
  gfx::Rect bounds;

  DraggableRegion();
};

}  // namespace exo_shell

#endif  // EXO_SHELL_COMMON_DRAGGABLE_REGION_H_
