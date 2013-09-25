// Copyright (c) 2013 Stanislas Polu.
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
  
Handle<String> 
WrapSource(
    Handle<String> source) 
{
  HandleScope handle_scope;
  Handle<String> left =
    String::New("(function(root, exports) {");
  Handle<String> right = String::New("\n })");
  return handle_scope.Close(
      String::Concat(left, String::Concat(source, right)));
}

void 
RequireFromResource(
    Handle<Object> root,
    Handle<Object> exports,
    Handle<String> name,
    int resource_id) 
{
  HandleScope scope;

  Handle<String> source = String::NewExternal(
      new StaticV8ExternalAsciiStringResource(
        GetStringResource(resource_id)));
  Handle<String> wrapped_source = WrapSource(source);

  Handle<Script> script(Script::New(wrapped_source, name));
  Handle<Function> func = Handle<Function>::Cast(script->Run());
  Handle<Value> args[] = { root, exports };
  func->Call(root, 2, args);
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
ApiBindings::GetNativeFunction(
    Handle<String> name)
{
  if (name->Equals(v8::String::New("RequireExoBrowser")))
    return v8::FunctionTemplate::New(RequireExoBrowser);

  NOTREACHED() << "Non-existing function in `ApiBindings`: "
               << *v8::String::Utf8Value(name);
  return v8::FunctionTemplate::New();
}

void
ApiBindings::RequireExoBrowser(
    const v8::FunctionCallbackInfo<Value>& args)
{
  /* Lazy Initialization */
  Local<String> ExoBrowserSymbol = String::NewSymbol("exo_browser");
  Local<Value> ExoBrowserCachedExports = args.This()->Get(ExoBrowserSymbol);
  if (ExoBrowserCachedExports->IsObject()) {
    args.GetReturnValue().Set(ExoBrowserCachedExports);
    return;
  }

  Local<Object> ExoBrowserExports = Object::New();
  args.This()->Set(ExoBrowserSymbol, ExoBrowserExports);

  ExoSessionWrap::Init(ExoBrowserExports);
  ExoBrowserWrap::Init(ExoBrowserExports);
  ExoFrameWrap::Init(ExoBrowserExports);
  
  args.GetReturnValue().Set(ExoBrowserExports);
}

} // namespace exo_browser
