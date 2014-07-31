// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/renderer/extensions/local_source_map.h"

#include "src/renderer/extensions/static_v8_external_string_resource.h"

namespace extensions {

LocalSourceMap::LocalSourceMap()
{
}

LocalSourceMap::~LocalSourceMap() 
{
}

void LocalSourceMap::RegisterSource(
    const std::string& name,
    const std::string& source)
{
  source_map_[name] = source;
}

v8::Handle<v8::Value> 
LocalSourceMap::GetSource(
    v8::Isolate* isolate,
    const std::string& name) 
{
  if (!Contains(name))
    return v8::Undefined(isolate);
  return ConvertString(isolate, source_map_[name]);
}

bool 
LocalSourceMap::Contains(
    const std::string& name) 
{
  return !!source_map_.count(name);
}

v8::Handle<v8::String> 
LocalSourceMap::ConvertString(
    v8::Isolate* isolate,
    const base::StringPiece& string) 
{
  // v8 takes ownership of the StaticV8ExternalAsciiStringResource (see
  // v8::String::NewExternal()).
  return v8::String::NewExternal(
      isolate, new StaticV8ExternalAsciiStringResource(string));
}

} // namespace extensions
