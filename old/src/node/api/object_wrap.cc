// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "exo_browser/src/node/api/object_wrap.h"

namespace exo_browser {

ObjectWrap::ObjectWrap()
{
  this->AddRef();
  refs_ = 0;
}

ObjectWrap::~ObjectWrap()
{
  if (persistent().IsEmpty()) return;
  DCHECK(persistent().IsNearDeath());
  persistent().ClearWeak();
  persistent().Reset();
}

void 
ObjectWrap::Ref() 
{
  DCHECK(!persistent().IsEmpty());
  persistent().ClearWeak();
  refs_++;
}

void 
ObjectWrap::Unref() 
{
  DCHECK(!persistent().IsEmpty());
  DCHECK(!persistent().IsWeak());
  DCHECK(refs_ > 0);
  if(--refs_ == 0) MakeWeak();
}

void 
ObjectWrap::EmptyCallback(
    v8::Persistent<v8::Function>* cb_p) {
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  cb->Call(obj, 0, NULL);
  cb_p->Reset();
  delete cb_p;
}

void 
ObjectWrap::IntCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const int* integer) 
{
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  v8::Local<v8::Integer> i = v8::Integer::New(v8::Isolate::GetCurrent(), 
                                              *integer);
  v8::Local<v8::Value> argv[1] = { i };
  cb->Call(obj, 1, argv);
  cb_p->Reset();
  delete cb_p;
  delete integer;
}

void 
ObjectWrap::DoubleCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const double* number) 
{
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  v8::Local<v8::Number> d = v8::Number::New(v8::Isolate::GetCurrent(), *number);
  v8::Local<v8::Value> argv[1] = { d };
  cb->Call(obj, 1, argv);
  cb_p->Reset();
  delete cb_p;
  delete number;
}

void 
ObjectWrap::BooleanCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const bool* boolean) 
{
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  v8::Local<v8::Boolean> b = v8::Boolean::New(v8::Isolate::GetCurrent(), 
                                              *boolean);
  v8::Local<v8::Value> argv[1] = { b };
  cb->Call(obj, 1, argv);
  cb_p->Reset();
  delete cb_p;
  delete boolean;
}

void 
ObjectWrap::StringCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const std::string* str) 
{
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  v8::Local<v8::String> s = v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), 
                                                    str->c_str());
  v8::Local<v8::Value> argv[1] = { s };
  cb->Call(obj, 1, argv);
  cb_p->Reset();
  delete cb_p;
  delete str;
}

void 
ObjectWrap::PointCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const gfx::Point* point) 
{
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  v8::Local<v8::Array> point_array = v8::Array::New(v8::Isolate::GetCurrent());
  point_array->Set(0, v8::Integer::New(v8::Isolate::GetCurrent(), point->x()));
  point_array->Set(1, v8::Integer::New(v8::Isolate::GetCurrent(), point->y()));
  v8::Local<v8::Value> argv[1] = { point_array };
  cb->Call(obj, 1, argv);
  cb_p->Reset();
  delete cb_p;
  delete point;
}

void
ObjectWrap::SizeCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const gfx::Size* size) 
{
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  v8::Local<v8::Array> size_array = v8::Array::New(v8::Isolate::GetCurrent());
  size_array->Set(0, v8::Integer::New(v8::Isolate::GetCurrent(), 
                                      size->width()));
  size_array->Set(1, v8::Integer::New(v8::Isolate::GetCurrent(), 
                                      size->height()));
  v8::Local<v8::Value> argv[1] = { size_array };
  cb->Call(obj, 1, argv);
  cb_p->Reset();
  delete cb_p;
  delete size;
}

void 
ObjectWrap::PersistentCallback(
    v8::Persistent<v8::Function>* cb_p, 
    v8::Persistent<v8::Object>* arg_p)
{
  v8::HandleScope handle_scope(v8::Isolate::GetCurrent());
  v8::Local<v8::Function> cb = 
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), *cb_p);
  v8::Local<v8::Object> arg_o = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), *arg_p);
  v8::Local<v8::Object> obj = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  v8::Local<v8::Value> argv[1] = { arg_o };
  cb->Call(obj, 1, argv);
  cb_p->Reset();
  delete cb_p;
  arg_p->Reset();
  delete arg_p;
}

} // namespace exo_browser

