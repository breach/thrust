// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/renderer/extensions/native_handler.h"

namespace extensions {

NativeHandler::NativeHandler() : is_valid_(true) {}

NativeHandler::~NativeHandler() {}

void NativeHandler::Invalidate() {
  is_valid_ = false;
}

}  // namespace extensions
