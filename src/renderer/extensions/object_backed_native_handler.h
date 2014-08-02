// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_EXTENSIONS_OBJECT_BACKED_NATIVE_HANDLER_H_
#define CHROME_RENDERER_EXTENSIONS_OBJECT_BACKED_NATIVE_HANDLER_H_

#include <string>
#include <vector>

#include "base/bind.h"
#include "base/memory/linked_ptr.h"
#include "v8/include/v8.h"

#include "src/renderer/extensions/native_handler.h"
#include "src/renderer/extensions/scoped_persistent.h"
#include "src/renderer/extensions/unsafe_persistent.h"

namespace extensions {
class Context;

// An ObjectBackedNativeHandler is a factory for JS objects with functions on
// them that map to native C++ functions. Subclasses should call RouteFunction()
// in their constructor to define functions on the created JS objects.
class ObjectBackedNativeHandler : public NativeHandler {
 public:
  explicit ObjectBackedNativeHandler(Context* context);
  virtual ~ObjectBackedNativeHandler();

  // Create an object with bindings to the native functions defined through
  // RouteFunction().
  virtual v8::Handle<v8::Object> NewInstance() OVERRIDE;

  v8::Isolate* GetIsolate() const;

 protected:
  typedef base::Callback<void(const v8::FunctionCallbackInfo<v8::Value>&)>
      HandlerFunction;

  // Installs a new 'route' from |name| to |handler_function|. This means that
  // NewInstance()s of this ObjectBackedNativeHandler will have a property
  // |name| which will be handled by |handler_function|.
  void RouteFunction(const std::string& name,
                     const HandlerFunction& handler_function);

  Context* context() const { return context_; }

  virtual void Invalidate() OVERRIDE;

 private:
  // Callback for RouteFunction which routes the V8 call to the correct
  // base::Bound callback.
  static void Router(const v8::FunctionCallbackInfo<v8::Value>& args);

  // When RouteFunction is called we create a v8::Object to hold the data we
  // need when handling it in Router() - this is the base::Bound function to
  // route to.
  //
  // We need a v8::Object because it's possible for v8 to outlive the
  // base::Bound function; the lifetime of an ObjectBackedNativeHandler is the
  // lifetime of webkit's involvement with it, not the life of the v8 context.
  // A scenario when v8 will outlive us is if a frame holds onto the
  // contentWindow of an iframe after it's removed.
  //
  // So, we use v8::Objects here to hold that data, effectively refcounting
  // the data. When |this| is destroyed we remove the base::Bound function from
  // the object to indicate that it shoudn't be called.
  //
  // Storing UnsafePersistents is safe here, because the corresponding
  // Persistent handle is created in RouteFunction(), and it keeps the data
  // pointed by the UnsafePersistent alive. It's not made weak or disposed, and
  // nobody else has access to it. The Persistent is then disposed in
  // Invalidate().
  typedef std::vector<UnsafePersistent<v8::Object> > RouterData;
  RouterData router_data_;

  Context* context_;

  ScopedPersistent<v8::ObjectTemplate> object_template_;

  DISALLOW_COPY_AND_ASSIGN(ObjectBackedNativeHandler);
};

}  // namespace extensions

#endif  // CHROME_RENDERER_EXTENSIONS_OBJECT_BACKED_NATIVE_HANDLER_H_
