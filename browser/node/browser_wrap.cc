// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/browser_wrap.h"

#include "v8/include/v8.h"

using namespace v8;

namespace breach {

BrowserWrap::BrowserWrap()
{
}

BrowserWrap::~BrowserWrap()
{
}

Handle<Value> BrowserWrap::New(const Arguments& args) {
  HandleScope scope;

  BrowserWrap* browser_wrap = new BrowserWrap();
  obj->Wrap(args.This());

  return args.This();
}

void BrowserWrap::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Browser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  //tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
  //    FunctionTemplate::New(PlusOne)->GetFunction());

  Persistent<Function> constructor = 
    Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Browser"), constructor);
}
    
} // namespace breach

