// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "exo/exo_browser/node/api/object_wrap.h"

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
  persistent().Dispose();
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
  v8::Local<v8::Object> browser_o = 
    v8::Local<v8::Object>::New(v8::Isolate::GetCurrent(), 
        this->persistent());
  cb->Call(browser_o, 0, NULL);
  cb_p->Dispose();
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

void 
ObjectWrap::StringCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const std::string* str) 
{
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

void 
ObjectWrap::PointCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const gfx::Point* point) 
{
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

void
ObjectWrap::SizeCallback(
    v8::Persistent<v8::Function>* cb_p, 
    const gfx::Size* size) 
{
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

} // namespace exo_browser

