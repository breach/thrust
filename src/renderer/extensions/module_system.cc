// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/renderer/extensions/module_system.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/debug/trace_event.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "content/public/renderer/render_view.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"

#include "src/renderer/extensions/script_context.h"
#include "src/renderer/extensions/safe_builtins.h"
#include "src/renderer/extensions/console.h"

namespace extensions {

namespace {

const char* kModuleSystem = "module_system";
const char* kModuleName = "module_name";
const char* kModuleField = "module_field";
const char* kModulesField = "modules";

// Logs a fatal error for the calling context, with some added metadata about
// the context:
//  - Its type (blessed, unblessed, etc).
//  - Whether it's valid.
//  - The extension ID, if one exists.
//
// This will only actual be fatal in in dev/canary, since in too many cases
// we're at the mercy of the extension or web page's environment. They can mess
// up our JS in unexpected ways. Hopefully dev/canary channel will pick up such
// problems, but given the wider variety on stable/beta it's impossible to know.
void Fatal(ScriptContext* context, const std::string& message) {
  // Prepend some context metadata.
  std::string full_message = "(";
  if (!context->is_valid())
    full_message += "Invalid ";
  full_message += " context";
  full_message += ") ";
  full_message += message;

  console::Error(context->isolate()->GetCallingContext(), full_message);
}

void Warn(v8::Isolate* isolate, const std::string& message) {
  console::Warn(isolate->GetCallingContext(), message);
}

// Default exception handler which logs the exception.
class DefaultExceptionHandler : public ModuleSystem::ExceptionHandler {
 public:
  explicit DefaultExceptionHandler(ScriptContext* context)
      : context_(context) {}

  // Fatally dumps the debug info from |try_catch| to the console.
  // Make sure this is never used for exceptions that originate in external
  // code!
  virtual void HandleUncaughtException(const v8::TryCatch& try_catch) OVERRIDE {
    v8::HandleScope handle_scope(context_->isolate());
    std::string stack_trace = "<stack trace unavailable>";
    if (!try_catch.StackTrace().IsEmpty()) {
      v8::String::Utf8Value stack_value(try_catch.StackTrace());
      if (*stack_value)
        stack_trace.assign(*stack_value, stack_value.length());
      else
        stack_trace = "<could not convert stack trace to string>";
    }
    Fatal(context_, CreateExceptionString(try_catch) + "{" + stack_trace + "}");
  }

 private:
  ScriptContext* context_;
};

} // namespace

std::string ModuleSystem::ExceptionHandler::CreateExceptionString(
    const v8::TryCatch& try_catch) {
  v8::Handle<v8::Message> message(try_catch.Message());
  if (message.IsEmpty()) {
    return "try_catch has no message";
  }

  std::string resource_name = "<unknown resource>";
  if (!message->GetScriptResourceName().IsEmpty()) {
    v8::String::Utf8Value resource_name_v8(
        message->GetScriptResourceName()->ToString());
    resource_name.assign(*resource_name_v8, resource_name_v8.length());
  }

  std::string error_message = "<no error message>";
  if (!message->Get().IsEmpty()) {
    v8::String::Utf8Value error_message_v8(message->Get());
    error_message.assign(*error_message_v8, error_message_v8.length());
  }

  return base::StringPrintf("%s:%d: %s",
                            resource_name.c_str(),
                            message->GetLineNumber(),
                            error_message.c_str());
}

ModuleSystem::ModuleSystem(ScriptContext* context, SourceMap* source_map)
    : ObjectBackedNativeHandler(context),
      context_(context),
      source_map_(source_map),
      natives_enabled_(0),
      exception_handler_(new DefaultExceptionHandler(context)) {
  RouteFunction("require",
      base::Bind(&ModuleSystem::RequireForJs, base::Unretained(this)));
  RouteFunction("requireNative",
      base::Bind(&ModuleSystem::RequireNative, base::Unretained(this)));
  RouteFunction("privates",
      base::Bind(&ModuleSystem::Private, base::Unretained(this)));

  v8::Handle<v8::Object> global(context->v8_context()->Global());
  v8::Isolate* isolate = context->isolate();
  global->SetHiddenValue(
      v8::String::NewFromUtf8(isolate, kModulesField),
      v8::Object::New(isolate));
  global->SetHiddenValue(
      v8::String::NewFromUtf8(isolate, kModuleSystem),
      v8::External::New(isolate, this));
}

ModuleSystem::~ModuleSystem() {
  Invalidate();
}

void ModuleSystem::Invalidate() {
  if (!is_valid())
    return;

  // Clear the module system properties from the global context. It's polite,
  // and we use this as a signal in lazy handlers that we no longer exist.
  {
    v8::HandleScope scope(GetIsolate());
    v8::Handle<v8::Object> global = context()->v8_context()->Global();
    global->DeleteHiddenValue(
        v8::String::NewFromUtf8(GetIsolate(), kModulesField));
    global->DeleteHiddenValue(
        v8::String::NewFromUtf8(GetIsolate(), kModuleSystem));
  }

  // Invalidate all of the successfully required handlers we own.
  for (NativeHandlerMap::iterator it = native_handler_map_.begin();
       it != native_handler_map_.end(); ++it) {
    it->second->Invalidate();
  }

  ObjectBackedNativeHandler::Invalidate();
}

ModuleSystem::NativesEnabledScope::NativesEnabledScope(
    ModuleSystem* module_system)
    : module_system_(module_system) {
  module_system_->natives_enabled_++;
}

ModuleSystem::NativesEnabledScope::~NativesEnabledScope() {
  module_system_->natives_enabled_--;
  CHECK_GE(module_system_->natives_enabled_, 0);
}

void ModuleSystem::HandleException(const v8::TryCatch& try_catch) {
  exception_handler_->HandleUncaughtException(try_catch);
}

v8::Handle<v8::Value> ModuleSystem::Require(const std::string& module_name) {
  v8::EscapableHandleScope handle_scope(GetIsolate());
  return handle_scope.Escape(RequireForJsInner(
      v8::String::NewFromUtf8(GetIsolate(), module_name.c_str())));
}

void ModuleSystem::RequireForJs(
  const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Handle<v8::String> module_name = args[0]->ToString();
  args.GetReturnValue().Set(RequireForJsInner(module_name));
}

v8::Local<v8::Value> ModuleSystem::RequireForJsInner(
    v8::Handle<v8::String> module_name) {
  v8::EscapableHandleScope handle_scope(GetIsolate());
  v8::Context::Scope context_scope(context()->v8_context());

  v8::Handle<v8::Object> global(context()->v8_context()->Global());

  // The module system might have been deleted. This can happen if a different
  // context keeps a reference to us, but our frame is destroyed (e.g.
  // background page keeps reference to chrome object in a closed popup).
  v8::Handle<v8::Value> modules_value = global->GetHiddenValue(
      v8::String::NewFromUtf8(GetIsolate(), kModulesField));
  if (modules_value.IsEmpty() || modules_value->IsUndefined()) {
    Warn(GetIsolate(), "Extension view no longer exists");
    return v8::Undefined(GetIsolate());
  }

  v8::Handle<v8::Object> modules(v8::Handle<v8::Object>::Cast(modules_value));
  v8::Local<v8::Value> exports(modules->Get(module_name));
  if (!exports->IsUndefined())
    return handle_scope.Escape(exports);

  std::string module_name_str = *v8::String::Utf8Value(module_name);
  v8::Handle<v8::Value> source(GetSource(module_name_str));
  if (source.IsEmpty() || source->IsUndefined()) {
    Fatal(context_, "No source for require(" + module_name_str + ")");
    return v8::Undefined(GetIsolate());
  }
  v8::Handle<v8::String> wrapped_source(WrapSource(
      v8::Handle<v8::String>::Cast(source)));
  // Modules are wrapped in (function(){...}) so they always return functions.
  v8::Handle<v8::Value> func_as_value = RunString(wrapped_source, module_name);
  if (func_as_value.IsEmpty() || func_as_value->IsUndefined()) {
    Fatal(context_, "Bad source for require(" + module_name_str + ")");
    return v8::Undefined(GetIsolate());
  }

  v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(func_as_value);

  exports = v8::Object::New(GetIsolate());
  v8::Handle<v8::Object> natives(NewInstance());
  CHECK(!natives.IsEmpty());  // this can happen if v8 has issues

  // These must match the argument order in WrapSource.
  v8::Handle<v8::Value> args[] = {
      // CommonJS.
      natives->Get(v8::String::NewFromUtf8(
          GetIsolate(), "require", v8::String::kInternalizedString)),
      natives->Get(v8::String::NewFromUtf8(
          GetIsolate(), "requireNative", v8::String::kInternalizedString)),
      exports,
      // Libraries that we magically expose to every module.
      console::AsV8Object(),
      natives->Get(v8::String::NewFromUtf8(
          GetIsolate(), "privates", v8::String::kInternalizedString)),
      // Each safe builtin. Keep in order with the arguments in WrapSource.
      context_->safe_builtins()->GetArray(),
      context_->safe_builtins()->GetFunction(),
      context_->safe_builtins()->GetJSON(),
      context_->safe_builtins()->GetObjekt(),
      context_->safe_builtins()->GetRegExp(),
      context_->safe_builtins()->GetString(), };
  {
    v8::TryCatch try_catch;
    try_catch.SetCaptureMessage(true);
    context_->CallFunction(func, arraysize(args), args);
    if (try_catch.HasCaught()) {
      HandleException(try_catch);
      return v8::Undefined(GetIsolate());
    }
  }
  modules->Set(module_name, exports);
  return handle_scope.Escape(exports);
}

v8::Local<v8::Value> ModuleSystem::CallModuleMethod(
    const std::string& module_name,
    const std::string& method_name) {
  v8::HandleScope handle_scope(GetIsolate());
  v8::Handle<v8::Value> no_args;
  return CallModuleMethod(module_name, method_name, 0, &no_args);
}

v8::Local<v8::Value> ModuleSystem::CallModuleMethod(
    const std::string& module_name,
    const std::string& method_name,
    std::vector<v8::Handle<v8::Value> >* args) {
  return CallModuleMethod(
      module_name, method_name, args->size(), vector_as_array(args));
}

v8::Local<v8::Value> ModuleSystem::CallModuleMethod(
    const std::string& module_name,
    const std::string& method_name,
    int argc,
    v8::Handle<v8::Value> argv[]) {
  TRACE_EVENT2("v8", "v8.callModuleMethod",
               "module_name", module_name,
               "method_name", method_name);

  v8::EscapableHandleScope handle_scope(GetIsolate());
  v8::Context::Scope context_scope(context()->v8_context());

  v8::Local<v8::Value> module;
  {
    NativesEnabledScope natives_enabled(this);
    module = RequireForJsInner(
        v8::String::NewFromUtf8(GetIsolate(), module_name.c_str()));
  }

  if (module.IsEmpty() || !module->IsObject()) {
    Fatal(context_,
          "Failed to get module " + module_name + " to call " + method_name);
    return handle_scope.Escape(
        v8::Local<v8::Primitive>(v8::Undefined(GetIsolate())));
  }

  v8::Local<v8::Value> value =
      v8::Handle<v8::Object>::Cast(module)->Get(
          v8::String::NewFromUtf8(GetIsolate(), method_name.c_str()));
  if (value.IsEmpty() || !value->IsFunction()) {
    Fatal(context_, module_name + "." + method_name + " is not a function");
    return handle_scope.Escape(
        v8::Local<v8::Primitive>(v8::Undefined(GetIsolate())));
  }

  v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(value);
  v8::Local<v8::Value> result;
  {
    v8::TryCatch try_catch;
    try_catch.SetCaptureMessage(true);
    result = context_->CallFunction(func, argc, argv);
    if (try_catch.HasCaught())
      HandleException(try_catch);
  }
  return handle_scope.Escape(result);
}

void ModuleSystem::RegisterNativeHandler(const std::string& name,
    scoped_ptr<NativeHandler> native_handler) {
  native_handler_map_[name] =
      linked_ptr<NativeHandler>(native_handler.release());
}

void ModuleSystem::OverrideNativeHandlerForTest(const std::string& name) {
  overridden_native_handlers_.insert(name);
}

void ModuleSystem::RunString(const std::string& code, const std::string& name) {
  v8::HandleScope handle_scope(GetIsolate());
  RunString(v8::String::NewFromUtf8(GetIsolate(), code.c_str()),
            v8::String::NewFromUtf8(GetIsolate(), name.c_str()));
}

// static
void ModuleSystem::NativeLazyFieldGetter(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  LazyFieldGetterInner(property,
                       info,
                       &ModuleSystem::RequireNativeFromString);
}

// static
void ModuleSystem::LazyFieldGetter(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  LazyFieldGetterInner(property, info, &ModuleSystem::Require);
}

// static
void ModuleSystem::LazyFieldGetterInner(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info,
    RequireFunction require_function) {
  CHECK(!info.Data().IsEmpty());
  CHECK(info.Data()->IsObject());
  v8::HandleScope handle_scope(info.GetIsolate());
  v8::Handle<v8::Object> parameters = v8::Handle<v8::Object>::Cast(info.Data());
  // This context should be the same as context()->v8_context().
  v8::Handle<v8::Context> context = parameters->CreationContext();
  v8::Handle<v8::Object> global(context->Global());
  v8::Handle<v8::Value> module_system_value = global->GetHiddenValue(
      v8::String::NewFromUtf8(info.GetIsolate(), kModuleSystem));
  if (module_system_value.IsEmpty() || !module_system_value->IsExternal()) {
    // ModuleSystem has been deleted.
    // TODO(kalman): See comment in header file.
    Warn(info.GetIsolate(),
         "Module system has been deleted, does extension view exist?");
    return;
  }

  ModuleSystem* module_system = static_cast<ModuleSystem*>(
      v8::Handle<v8::External>::Cast(module_system_value)->Value());

  std::string name = *v8::String::Utf8Value(parameters->Get(
      v8::String::NewFromUtf8(info.GetIsolate(), kModuleName))->ToString());

  // Switch to our v8 context because we need functions created while running
  // the require()d module to belong to our context, not the current one.
  v8::Context::Scope context_scope(context);
  NativesEnabledScope natives_enabled_scope(module_system);

  v8::TryCatch try_catch;
  v8::Handle<v8::Value> module_value = (module_system->*require_function)(name);
  if (try_catch.HasCaught()) {
    module_system->HandleException(try_catch);
    return;
  }
  if (module_value.IsEmpty() || !module_value->IsObject()) {
    // require_function will have already logged this, we don't need to.
    return;
  }

  v8::Handle<v8::Object> module = v8::Handle<v8::Object>::Cast(module_value);
  v8::Handle<v8::String> field =
      parameters->Get(v8::String::NewFromUtf8(info.GetIsolate(), kModuleField))
          ->ToString();

  if (!module->Has(field)) {
    std::string field_str = *v8::String::Utf8Value(field);
    Fatal(module_system->context_,
          "Lazy require of " + name + "." + field_str + " did not set the " +
              field_str + " field");
    return;
  }

  v8::Local<v8::Value> new_field = module->Get(field);
  if (try_catch.HasCaught()) {
    module_system->HandleException(try_catch);
    return;
  }

  // Ok for it to be undefined, among other things it's how bindings signify
  // that the extension doesn't have permission to use them.
  CHECK(!new_field.IsEmpty());

  // Delete the getter and set this field to |new_field| so the same object is
  // returned every time a certain API is accessed.
  v8::Handle<v8::Object> object = info.This();
  object->Delete(property);
  object->Set(property, new_field);
  info.GetReturnValue().Set(new_field);
}

void ModuleSystem::SetLazyField(v8::Handle<v8::Object> object,
                                const std::string& field,
                                const std::string& module_name,
                                const std::string& module_field) {
  SetLazyField(object, field, module_name, module_field,
      &ModuleSystem::LazyFieldGetter);
}

void ModuleSystem::SetLazyField(v8::Handle<v8::Object> object,
                                const std::string& field,
                                const std::string& module_name,
                                const std::string& module_field,
                                v8::AccessorGetterCallback getter) {
  v8::HandleScope handle_scope(GetIsolate());
  v8::Handle<v8::Object> parameters = v8::Object::New(GetIsolate());
  parameters->Set(v8::String::NewFromUtf8(GetIsolate(), kModuleName),
                  v8::String::NewFromUtf8(GetIsolate(), module_name.c_str()));
  parameters->Set(v8::String::NewFromUtf8(GetIsolate(), kModuleField),
                  v8::String::NewFromUtf8(GetIsolate(), module_field.c_str()));
  object->SetAccessor(v8::String::NewFromUtf8(GetIsolate(), field.c_str()),
                      getter,
                      NULL,
                      parameters);
}

void ModuleSystem::SetNativeLazyField(v8::Handle<v8::Object> object,
                                      const std::string& field,
                                      const std::string& module_name,
                                      const std::string& module_field) {
  SetLazyField(object, field, module_name, module_field,
      &ModuleSystem::NativeLazyFieldGetter);
}


v8::Handle<v8::Value> ModuleSystem::RunString(v8::Handle<v8::String> code,
                                              v8::Handle<v8::String> name) {
  v8::EscapableHandleScope handle_scope(GetIsolate());
  v8::Context::Scope context_scope(context()->v8_context());

  // Prepend extensions:: to |name| so that internal code can be differentiated
  // from external code in stack traces. This has no effect on behaviour.
  std::string internal_name = base::StringPrintf("extensions::%s",
                                                 *v8::String::Utf8Value(name));

  blink::WebScopedMicrotaskSuppression suppression;
  v8::TryCatch try_catch;
  try_catch.SetCaptureMessage(true);
  v8::Handle<v8::Script> script(
      v8::Script::Compile(code,
                          v8::String::NewFromUtf8(GetIsolate(),
                                                  internal_name.c_str(),
                                                  v8::String::kNormalString,
                                                  internal_name.size())));
  if (try_catch.HasCaught()) {
    HandleException(try_catch);
    return v8::Undefined(GetIsolate());
  }

  v8::Local<v8::Value> result = script->Run();
  if (try_catch.HasCaught()) {
    HandleException(try_catch);
    return v8::Undefined(GetIsolate());
  }

  return handle_scope.Escape(result);
}

v8::Handle<v8::Value> ModuleSystem::GetSource(const std::string& module_name) {
  v8::EscapableHandleScope handle_scope(GetIsolate());
  if (!source_map_->Contains(module_name))
    return v8::Undefined(GetIsolate());
  return handle_scope.Escape(
      v8::Local<v8::Value>(source_map_->GetSource(GetIsolate(), module_name)));
}

void ModuleSystem::RequireNative(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  CHECK_EQ(1, args.Length());
  std::string native_name = *v8::String::Utf8Value(args[0]->ToString());
  args.GetReturnValue().Set(RequireNativeFromString(native_name));
}

v8::Handle<v8::Value> ModuleSystem::RequireNativeFromString(
    const std::string& native_name) {
  if (natives_enabled_ == 0) {
    // HACK: if in test throw exception so that we can test the natives-disabled
    // logic; however, under normal circumstances, this is programmer error so
    // we could crash.
    if (exception_handler_) {
      return GetIsolate()->ThrowException(
          v8::String::NewFromUtf8(GetIsolate(), "Natives disabled"));
    }
    Fatal(context_, "Natives disabled for requireNative(" + native_name + ")");
    return v8::Undefined(GetIsolate());
  }

  if (overridden_native_handlers_.count(native_name) > 0u) {
    return RequireForJsInner(
        v8::String::NewFromUtf8(GetIsolate(), native_name.c_str()));
  }

  NativeHandlerMap::iterator i = native_handler_map_.find(native_name);
  if (i == native_handler_map_.end()) {
    Fatal(context_,
          "Couldn't find native for requireNative(" + native_name + ")");
    return v8::Undefined(GetIsolate());
  }
  return i->second->NewInstance();
}

v8::Handle<v8::String> ModuleSystem::WrapSource(v8::Handle<v8::String> source) {
  v8::EscapableHandleScope handle_scope(GetIsolate());
  // Keep in order with the arguments in RequireForJsInner.
  v8::Handle<v8::String> left = v8::String::NewFromUtf8(
      GetIsolate(),
      "(function(require, requireNative, exports, "
      "console, privates,"
      "$Array, $Function, $JSON, $Object, $RegExp, $String) {"
      "'use strict';");
  v8::Handle<v8::String> right = v8::String::NewFromUtf8(GetIsolate(), "\n})");
  return handle_scope.Escape(v8::Local<v8::String>(
      v8::String::Concat(left, v8::String::Concat(source, right))));
}

void ModuleSystem::Private(const v8::FunctionCallbackInfo<v8::Value>& args) {
  CHECK_EQ(1, args.Length());
  CHECK(args[0]->IsObject());
  v8::Local<v8::Object> obj = args[0].As<v8::Object>();
  v8::Local<v8::String> privates_key =
      v8::String::NewFromUtf8(GetIsolate(), "privates");
  v8::Local<v8::Value> privates = obj->GetHiddenValue(privates_key);
  if (privates.IsEmpty()) {
    privates = v8::Object::New(args.GetIsolate());
    obj->SetHiddenValue(privates_key, privates);
  }
  args.GetReturnValue().Set(privates);
}

}  // namespace extensions
