// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) Joyent, Inc. and other Node contributors.
// See the LICENSE file.

#ifndef EXO_BROWSER_NODE_API_OBJECT_WRAP_H_
#define EXO_BROWSER_NODE_API_OBJECT_WRAP_H_

#include "base/memory/ref_counted.h"
#include "base/logging.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include <assert.h>
#include "third_party/node/src/node.h"
#include "v8/include/v8.h"

// Explicitly instantiate some template classes, so we're sure they will be
// present in the binary / shared object. There isn't much doubt that they will
// be, but MSVC tends to complain about these things.
#ifdef _MSC_VER
  template class NODE_EXTERN v8::Persistent<v8::Object>;
  template class NODE_EXTERN v8::Persistent<v8::FunctionTemplate>;
#endif

namespace exo_browser {

class ObjectWrap : public base::RefCountedThreadSafe<ObjectWrap> {
public:
  ObjectWrap();

  template <class T>
  static inline T* Unwrap(v8::Handle<v8::Object> handle) {
    DCHECK(!handle.IsEmpty());
    DCHECK(handle->InternalFieldCount() > 0);
    return static_cast<T*>(handle->GetAlignedPointerFromInternalField(0));
  }

  inline v8::Local<v8::Object> handle() {
    return handle(v8::Isolate::GetCurrent());
  }

  inline v8::Local<v8::Object> handle(v8::Isolate* isolate) {
    return v8::Local<v8::Object>::New(isolate, persistent());
  }

  inline v8::Persistent<v8::Object>& persistent() {
    return handle_;
  }

protected:
  virtual ~ObjectWrap(); 

  inline void Wrap(v8::Handle<v8::Object> handle) {
    DCHECK(persistent().IsEmpty());
    DCHECK(handle->InternalFieldCount() > 0);
    handle->SetAlignedPointerInInternalField(0, this);
    persistent().Reset(v8::Isolate::GetCurrent(), handle);
    MakeWeak();
  }

  inline void MakeWeak(void) {
    persistent().MakeWeak(this, WeakCallback);
    persistent().MarkIndependent();
  }

  virtual void Ref();
  virtual void Unref();

  int refs_;

  // Some Callbacks used by Subclasses

  void EmptyCallback(v8::Persistent<v8::Function>* cb_p);

  void IntCallback(v8::Persistent<v8::Function>* cb_p, 
                   const int* integer);

  void BooleanCallback(v8::Persistent<v8::Function>* cb_p, 
                       const bool* boolean);

  void StringCallback(v8::Persistent<v8::Function>* cb_p, 
                      const std::string* str);

  void PointCallback(v8::Persistent<v8::Function>* cb_p, 
                     const gfx::Point* point);

  void SizeCallback(v8::Persistent<v8::Function>* cb_p, 
                    const gfx::Size* size);

private:
  static void WeakCallback(v8::Isolate* isolate,
                           v8::Persistent<v8::Object>* pobj,
                           ObjectWrap* wrap) {
    v8::HandleScope scope(isolate);
    DCHECK(wrap->refs_ == 0);
    DCHECK(*pobj == wrap->persistent());
    DCHECK((*pobj).IsNearDeath());
    wrap->Release();
  }

  v8::Persistent<v8::Object> handle_;

  friend class base::RefCountedThreadSafe<ObjectWrap>;
};

} // namespace exo_browser

#endif // EXO_BROWSER_NODE_API_OBJECT_WRAP_H_
