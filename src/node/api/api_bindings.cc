// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "exo_browser/src/node/api/api_bindings.h"

#include "grit/exo_browser_resources.h"
#include "base/strings/string_piece.h"

#include "exo_browser/src/node/api/exo_session_wrap.h"
#include "exo_browser/src/node/api/exo_browser_wrap.h"
#include "exo_browser/src/node/api/exo_frame_wrap.h"

using namespace v8;

namespace {

class StaticV8ExternalAsciiStringResource
    : public v8::String::ExternalAsciiStringResource {
 public:
  explicit StaticV8ExternalAsciiStringResource(
      const base::StringPiece& buffer)
    : buffer_(buffer) {}

  virtual ~StaticV8ExternalAsciiStringResource() {};

  virtual const char* data() const OVERRIDE { return buffer_.data(); }
  virtual size_t length() const OVERRIDE { return buffer_.length(); }

 private:
  base::StringPiece buffer_;
};


base::StringPiece 
GetStringResource(
    int resource_id) 
{
  return ResourceBundle::GetSharedInstance().GetRawDataResource(resource_id);
}
  
} // namespace

namespace exo_browser {

ApiBindings::ApiBindings()
  : Extension("api_bindings.js",
               GetStringResource(IDR_EXO_BROWSER_API_BINDINGS_JS).data(),
               0,     // num dependencies.
               NULL,  // dependencies array.
               GetStringResource(IDR_EXO_BROWSER_API_BINDINGS_JS).size()) 
{
}

ApiBindings::~ApiBindings()
{
}

Handle<FunctionTemplate>
ApiBindings::GetNativeFunctionTemplate(
    Isolate *isolate,
    Handle<String> name)
{
  if (name->Equals(v8::String::NewFromUtf8(isolate, "RequireExoBrowser")))
    return v8::FunctionTemplate::New(isolate, RequireExoBrowser);

  NOTREACHED() << "Non-existing function in `ApiBindings`: "
               << *v8::String::Utf8Value(name);
  return v8::FunctionTemplate::New(isolate);
}

void
ApiBindings::RequireExoBrowser(
    const v8::FunctionCallbackInfo<Value>& args)
{
  Isolate* isolate = Isolate::GetCurrent();

  /* Lazy Initialization */
  Local<String> ExoBrowserSymbol = String::NewFromUtf8(isolate, "exo_browser");
  Local<Value> ExoBrowserCachedExports = args.This()->Get(ExoBrowserSymbol);
  if (ExoBrowserCachedExports->IsObject()) {
    args.GetReturnValue().Set(ExoBrowserCachedExports);
    return;
  }

  Local<Object> ExoBrowserExports = Object::New(isolate);
  args.This()->Set(ExoBrowserSymbol, ExoBrowserExports);

  ExoSessionWrap::Init(ExoBrowserExports);
  ExoBrowserWrap::Init(ExoBrowserExports);
  ExoFrameWrap::Init(ExoBrowserExports);
  
  args.GetReturnValue().Set(ExoBrowserExports);
}

} // namespace exo_browser
