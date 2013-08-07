// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) Joyent, Inc. and other Node contributors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_OBJECT_WRAP_H_
#define BREACH_BROWSER_NODE_API_OBJECT_WRAP_H_

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

namespace breach {

class ObjectWrap : public base::RefCountedThreadSafe<ObjectWrap> {
public:
  ObjectWrap() {
    this->AddRef();
    refs_ = 0;
  }

  virtual ~ObjectWrap() {
    if (persistent().IsEmpty()) return;
    DCHECK(persistent().IsNearDeath());
    persistent().ClearWeak();
    persistent().Dispose();
  }

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

  virtual void Ref() {
    DCHECK(!persistent().IsEmpty());
    persistent().ClearWeak();
    refs_++;
  }

  virtual void Unref() {
    DCHECK(!persistent().IsEmpty());
    DCHECK(!persistent().IsWeak());
    DCHECK(refs_ > 0);
    if(--refs_ == 0) MakeWeak();
  }

  int refs_;

  // Some Callbacks used by Subclasses

  void EmptyCallback(v8::Persistent<v8::Function>* cb_p) {
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
    v8::Local<v8::Function> cb = 
      v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
    v8::Local<v8::Object> browser_o = 
      v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
                                 this->persistent());
    cb->Call(browser_o, 0, NULL);
    cb_p->Dispose();
    delete cb_p;
  }

  void IntCallback(v8::Persistent<v8::Function>* cb_p, 
                   const int* integer) {
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
    v8::Local<v8::Function> cb = 
      v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
    v8::Local<v8::Object> browser_o = 
      v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
                                 this->persistent());
    v8::Local<v8::Integer> i = v8::Integer::New(*integer);
    v8::Local<v8::Value> argv[1] = { i };
    cb->Call(browser_o, 1, argv);
    cb_p->Dispose();
    delete cb_p;
    delete integer;
  }

  void StringCallback(v8::Persistent<v8::Function>* cb_p, 
                      const std::string* str) {
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
    v8::Local<v8::Function> cb = 
      v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
    v8::Local<v8::Object> browser_o = 
      v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
                                 this->persistent());
    v8::Local<v8::String> s = v8::String::New(str->c_str());
    v8::Local<v8::Value> argv[1] = { s };
    cb->Call(browser_o, 1, argv);
    cb_p->Dispose();
    delete cb_p;
    delete str;
  }

  void PointCallback(v8::Persistent<v8::Function>* cb_p, 
                     const gfx::Point* point) {
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
    v8::Local<v8::Function> cb = 
      v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
    v8::Local<v8::Object> browser_o = 
      v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
                                 this->persistent());
    v8::Local<v8::Array> point_array = v8::Array::New();
    point_array->Set(0, v8::Integer::New(point->x()));
    point_array->Set(1, v8::Integer::New(point->y()));
    v8::Local<v8::Value> argv[1] = { point_array };
    cb->Call(browser_o, 1, argv);
    cb_p->Dispose();
    delete cb_p;
    delete point;
  }

  void SizeCallback(v8::Persistent<v8::Function>* cb_p, 
                    const gfx::Size* size) {
    v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
    v8::Local<v8::Function> cb = 
      v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
    v8::Local<v8::Object> browser_o = 
      v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
                                 this->persistent());
    v8::Local<v8::Array> size_array = v8::Array::New();
    size_array->Set(0, v8::Integer::New(size->width()));
    size_array->Set(1, v8::Integer::New(size->height()));
    v8::Local<v8::Value> argv[1] = { size_array };
    cb->Call(browser_o, 1, argv);
    cb_p->Dispose();
    delete cb_p;
    delete size;
  }

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
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_OBJECT_WRAP_H_
