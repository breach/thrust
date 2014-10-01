// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_EXTENSIONS_UNSAFE_PERSISTENT_H_
#define CHROME_RENDERER_EXTENSIONS_UNSAFE_PERSISTENT_H_

#include "v8/include/v8.h"

namespace extensions {

// An unsafe way to pass Persistent handles around. Do not use unless you know
// what you're doing. UnsafePersistent is only safe to use when we know that the
// memory pointed by it is not going away: 1) When GC cannot happen while the
// UnsafePersistent is alive or 2) when there is a strong Persistent keeping the
// memory alive while the UnsafePersistent is alive.
template<typename T> class UnsafePersistent {
 public:
  UnsafePersistent() : value_(0) { }

  explicit UnsafePersistent(v8::Persistent<T>* handle) {
    value_ = handle->ClearAndLeak();
  }

  UnsafePersistent(v8::Isolate* isolate, const v8::Handle<T>& handle) {
    v8::Persistent<T> persistent(isolate, handle);
    value_ = persistent.ClearAndLeak();
  }

  // Usage of this function requires
  // V8_ALLOW_ACCESS_TO_RAW_HANDLE_CONSTRUCTOR to be defined
  void dispose() {
    v8::Persistent<T> handle(value_);
    handle.Reset();
    value_ = 0;
  }

  // Usage of this function requires
  // V8_ALLOW_ACCESS_TO_RAW_HANDLE_CONSTRUCTOR to be defined
  v8::Local<T> newLocal(v8::Isolate* isolate) {
    return v8::Local<T>::New(isolate, v8::Local<T>(value_));
  }

 private:
  T* value_;
};

}  // namespace extensions

#endif  // CHROME_RENDERER_EXTENSIONS_UNSAFE_PERSISTENT_H_
