// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXO_SHELL_RENDERER_EXTENSIONS_STATIC_V8_EXTERNAL_STRING_RESOURCE_H_
#define EXO_SHELL_RENDERER_EXTENSIONS_STATIC_V8_EXTERNAL_STRING_RESOURCE_H_

#include "base/compiler_specific.h"
#include "base/strings/string_piece.h"
#include "v8/include/v8.h"

namespace extensions {

// A very simple implementation of v8::ExternalAsciiStringResource that just
// wraps a buffer. The buffer must outlive the v8 runtime instance this resource
// is used in.
class StaticV8ExternalAsciiStringResource
    : public v8::String::ExternalAsciiStringResource {
 public:
  explicit StaticV8ExternalAsciiStringResource(const base::StringPiece& buffer);
  virtual ~StaticV8ExternalAsciiStringResource();

  virtual const char* data() const OVERRIDE;
  virtual size_t length() const OVERRIDE;

 private:
  base::StringPiece buffer_;
};

} // namspace extensions

#endif // EXO_SHELL_RENDERER_EXTENSIONS_STATIC_V8_EXTERNAL_STRING_RESOURCE_H_
