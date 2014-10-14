// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_EXTENSIONS_SAFE_BUILTINS_H_
#define CHROME_RENDERER_EXTENSIONS_SAFE_BUILTINS_H_

#include "v8/include/v8.h"

namespace extensions {
class ScriptContext;

// A collection of safe builtin objects, in that they won't be tained by
// extensions overriding methods on them.
class SafeBuiltins {
 public:
  // Creates the v8::Extension which manages SafeBuiltins instances.
  static v8::Extension* CreateV8Extension();

  explicit SafeBuiltins(ScriptContext* context);

  virtual ~SafeBuiltins();

  // Each method returns an object with methods taken from their respective
  // builtin object's prototype, adapted to automatically call() themselves.
  //
  // Examples:
  //   Array.prototype.forEach.call(...) becomes Array.forEach(...)
  //   Object.prototype.toString.call(...) becomes Object.toString(...)
  //   Object.keys.call(...) becomes Object.keys(...)
  v8::Local<v8::Object> GetArray() const;
  v8::Local<v8::Object> GetFunction() const;
  v8::Local<v8::Object> GetJSON() const;
  // NOTE(kalman): VS2010 won't compile "GetObject", it mysteriously renames it
  // to "GetObjectW" - hence GetObjekt. Sorry.
  v8::Local<v8::Object> GetObjekt() const;
  v8::Local<v8::Object> GetRegExp() const;
  v8::Local<v8::Object> GetString() const;

 private:
  ScriptContext* context_;
};

} //  namespace extensions

#endif  // CHROME_RENDERER_EXTENSIONS_SAFE_BUILTINS_H_
