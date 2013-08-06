// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/web_contents_wrap.h"

#include "content/public/browser/web_contents.h"
#include "breach/browser/node/api/exo_frame_wrap.h"
#include "breach/browser/node/api/exo_browser_wrap.h"

namespace breach {

Persistent<Function> WebContentsWrap::s_constructor;

void 
WebContentsWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("_WebContentsWrap"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  /* Prototype */

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());
}


WebContentsWrap::WebContentsWrap()
{
}

WebContentsWrap::~WebContentsWrap()
{
  LOG(INFO) << "WebContentsWrap Destructor";
  /* Nothing to do. WebContents is not owned by us. */
}

void 
WebContentsWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());

  WebContentsWrap* wrap = new WebContentsWrap();
  wrap->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

} // namespace breach

