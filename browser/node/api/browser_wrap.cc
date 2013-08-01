// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/browser_wrap.h"

using namespace v8;

namespace breach {

BrowserWrap::BrowserWrap()
{
}

BrowserWrap::~BrowserWrap()
{
}

void 
BrowserWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  BrowserWrap* browser_wrap = new BrowserWrap();
  browser_wrap->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void 
BrowserWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Browser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  //tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
  //    FunctionTemplate::New(PlusOne)->GetFunction());

  exports->Set(String::NewSymbol("Browser"), tpl->GetFunction());
}
    
} // namespace breach

