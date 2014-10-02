// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/renderer/extensions/safe_builtins.h"

#include "base/logging.h"
#include "base/stl_util.h"
#include "base/strings/stringprintf.h"
#include "base/basictypes.h"
#include "url/gurl.h"

#include "src/renderer/extensions/script_context.h"

namespace extensions {

namespace {

const char kClassName[] = "extensions::SafeBuiltins";

// Documentation for makeCallback in the JavaScript, out here to reduce the
// (very small) amount of effort that the v8 parser needs to do:
//
// Returns a new object with every function on |obj| configured to call()\n"
// itself with the given arguments.\n"
// E.g. given\n"
//    var result = makeCallable(Function.prototype)\n"
// |result| will be a object including 'bind' such that\n"
//    result.bind(foo, 1, 2, 3);\n"
// is equivalent to Function.prototype.bind.call(foo, 1, 2, 3), and so on.\n"
// This is a convenient way to save functions that user scripts may clobber.\n"
const char kScript[] =
    "(function() {\n"
    "'use strict';\n"
    "native function Apply();\n"
    "native function Save();\n"
    "\n"
    "// Used in the callback implementation, could potentially be clobbered.\n"
    "function makeCallable(obj, target, isStatic, propertyNames) {\n"
    "  propertyNames.forEach(function(propertyName) {\n"
    "    var property = obj[propertyName];\n"
    "    target[propertyName] = function() {\n"
    "      var recv = obj;\n"
    "      var firstArgIndex = 0;\n"
    "      if (!isStatic) {\n"
    "        if (arguments.length == 0)\n"
    "          throw 'There must be at least one argument, the recevier';\n"
    "        recv = arguments[0];\n"
    "        firstArgIndex = 1;\n"
    "      }\n"
    "      return Apply(\n"
    "          property, recv, arguments, firstArgIndex, arguments.length);\n"
    "    };\n"
    "  });\n"
    "}\n"
    "\n"
    "function saveBuiltin(builtin, protoPropertyNames, staticPropertyNames) {\n"
    "  var safe = function() {\n"
    "    throw 'Safe objects cannot be called nor constructed. ' +\n"
    "          'Use $Foo.self() or new $Foo.self() instead.';\n"
    "  };\n"
    "  safe.self = builtin;\n"
    "  makeCallable(builtin.prototype, safe, false, protoPropertyNames);\n"
    "  if (staticPropertyNames)\n"
    "    makeCallable(builtin, safe, true, staticPropertyNames);\n"
    "  Save(builtin.name, safe);\n"
    "}\n"
    "\n"
    "// Save only what is needed to make tests that override builtins pass.\n"
    "saveBuiltin(Object,\n"
    "            ['hasOwnProperty'],\n"
    "            ['create', 'defineProperty', 'getOwnPropertyDescriptor',\n"
    "             'getPrototypeOf', 'keys']);\n"
    "saveBuiltin(Function,\n"
    "            ['apply', 'bind', 'call']);\n"
    "saveBuiltin(Array,\n"
    "            ['concat', 'forEach', 'indexOf', 'join', 'push', 'slice',\n"
    "             'splice', 'map', 'filter']);\n"
    "saveBuiltin(String,\n"
    "            ['slice', 'split']);\n"
    "saveBuiltin(RegExp,\n"
    "            ['test']);\n"
    "\n"
    "// JSON is trickier because extensions can override toJSON in\n"
    "// incompatible ways, and we need to prevent that.\n"
    "var builtinTypes = [\n"
    "  Object, Function, Array, String, Boolean, Number, Date, RegExp\n"
    "];\n"
    "var builtinToJSONs = builtinTypes.map(function(t) {\n"
    "  return t.toJSON;\n"
    "});\n"
    "var builtinArray = Array;\n"
    "var builtinJSONStringify = JSON.stringify;\n"
    "Save('JSON', {\n"
    "  parse: JSON.parse,\n"
    "  stringify: function(obj) {\n"
    "    var savedToJSONs = new builtinArray(builtinTypes.length);\n"
    "    try {\n"
    "      for (var i = 0; i < builtinTypes.length; ++i) {\n"
    "        try {\n"
    "          if (builtinTypes[i].prototype.toJSON !==\n"
    "              builtinToJSONs[i]) {\n"
    "            savedToJSONs[i] = builtinTypes[i].prototype.toJSON;\n"
    "            builtinTypes[i].prototype.toJSON = builtinToJSONs[i];\n"
    "          }\n"
    "        } catch (e) {}\n"
    "      }\n"
    "    } catch (e) {}\n"
    "    try {\n"
    "      return builtinJSONStringify(obj);\n"
    "    } finally {\n"
    "      for (var i = 0; i < builtinTypes.length; ++i) {\n"
    "        try {\n"
    "          if (i in savedToJSONs)\n"
    "            builtinTypes[i].prototype.toJSON = savedToJSONs[i];\n"
    "        } catch (e) {}\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "});\n"
    "\n"
    "}());\n";

v8::Local<v8::String> MakeKey(const char* name, v8::Isolate* isolate) {
  return v8::String::NewFromUtf8(
      isolate, base::StringPrintf("%s::%s", kClassName, name).c_str());
}

void SaveImpl(const char* name,
              v8::Local<v8::Value> value,
              v8::Local<v8::Context> context) {
  CHECK(!value.IsEmpty() && value->IsObject()) << name;
  context->Global()
      ->SetHiddenValue(MakeKey(name, context->GetIsolate()), value);
}

v8::Local<v8::Object> Load(const char* name, v8::Handle<v8::Context> context) {
  v8::Local<v8::Value> value =
      context->Global()->GetHiddenValue(MakeKey(name, context->GetIsolate()));
  CHECK(!value.IsEmpty() && value->IsObject()) << name;
  return value->ToObject();
}

class ExtensionImpl : public v8::Extension {
 public:
  ExtensionImpl() : v8::Extension(kClassName, kScript) {}

 private:
  virtual v8::Handle<v8::FunctionTemplate> GetNativeFunctionTemplate(
      v8::Isolate* isolate,
      v8::Handle<v8::String> name) OVERRIDE {
    if (name->Equals(v8::String::NewFromUtf8(isolate, "Apply")))
      return v8::FunctionTemplate::New(isolate, Apply);
    if (name->Equals(v8::String::NewFromUtf8(isolate, "Save")))
      return v8::FunctionTemplate::New(isolate, Save);
    NOTREACHED() << *v8::String::Utf8Value(name);
    return v8::Handle<v8::FunctionTemplate>();
  }

  static void Apply(const v8::FunctionCallbackInfo<v8::Value>& info) {
    CHECK(info.Length() == 5 &&
          info[0]->IsFunction() &&  // function
          // info[1] could be an object or a string
          info[2]->IsObject() &&    // args
          info[3]->IsInt32() &&     // first_arg_index
          info[4]->IsInt32());      // args_length
    v8::Local<v8::Function> function = info[0].As<v8::Function>();
    v8::Local<v8::Object> recv;
    if (info[1]->IsObject()) {
      recv = info[1]->ToObject();
    } else if (info[1]->IsString()) {
      recv = v8::StringObject::New(info[1]->ToString())->ToObject();
    } else {
      info.GetIsolate()->ThrowException(
          v8::Exception::TypeError(v8::String::NewFromUtf8(
              info.GetIsolate(),
              "The first argument is the receiver and must be an object")));
      return;
    }
    v8::Local<v8::Object> args = info[2]->ToObject();
    int first_arg_index = static_cast<int>(info[3]->ToInt32()->Value());
    int args_length = static_cast<int>(info[4]->ToInt32()->Value());

    int argc = args_length - first_arg_index;
    scoped_ptr<v8::Local<v8::Value>[]> argv(new v8::Local<v8::Value>[argc]);
    for (int i = 0; i < argc; ++i) {
      CHECK(args->Has(i + first_arg_index));
      argv[i] = args->Get(i + first_arg_index);
    }

    v8::Local<v8::Value> return_value = function->Call(recv, argc, argv.get());
    if (!return_value.IsEmpty())
      info.GetReturnValue().Set(return_value);
  }

  static void Save(const v8::FunctionCallbackInfo<v8::Value>& info) {
    CHECK(info.Length() == 2 &&
          info[0]->IsString() &&
          info[1]->IsObject());
    SaveImpl(*v8::String::Utf8Value(info[0]),
             info[1],
             info.GetIsolate()->GetCallingContext());
  }
};

}  // namespace

// static
v8::Extension* SafeBuiltins::CreateV8Extension() {
  return new ExtensionImpl();
}

SafeBuiltins::SafeBuiltins(ScriptContext* context) : context_(context) {}

SafeBuiltins::~SafeBuiltins() {}

v8::Local<v8::Object> SafeBuiltins::GetArray() const {
  return Load("Array", context_->v8_context());
}

v8::Local<v8::Object> SafeBuiltins::GetFunction() const {
  return Load("Function", context_->v8_context());
}

v8::Local<v8::Object> SafeBuiltins::GetJSON() const {
  return Load("JSON", context_->v8_context());
}

v8::Local<v8::Object> SafeBuiltins::GetObjekt() const {
  return Load("Object", context_->v8_context());
}

v8::Local<v8::Object> SafeBuiltins::GetRegExp() const {
  return Load("RegExp", context_->v8_context());
}

v8::Local<v8::Object> SafeBuiltins::GetString() const {
  return Load("String", context_->v8_context());
}

} //  namespace extensions
