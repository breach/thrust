// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_RENDERER_EXTENSIONS_LOCAL_SOURCE_MAP_H_
#define EXO_SHELL_RENDERER_EXTENSIONS_LOCAL_SOURCE_MAP_H_

#include <map>
#include <string>

#include "base/strings/string_piece.h"
#include "v8/include/v8.h"

#include "src/renderer/extensions/module_system.h"

namespace extensions {

class LocalSourceMap : public ModuleSystem::SourceMap {
public:

  LocalSourceMap();
  ~LocalSourceMap();

  void RegisterSource(const std::string& name, const std::string& source);

  /****************************************************************************/
  /* SOURCEMAP IMPLEMENTATION */
  /****************************************************************************/
  virtual v8::Handle<v8::Value> GetSource(v8::Isolate* isolate,
                                          const std::string& name) OVERRIDE;
  virtual bool Contains(const std::string& name) OVERRIDE;

private:
  v8::Handle<v8::String> ConvertString(v8::Isolate* isolate,
                                       const base::StringPiece& string);

  std::map<std::string, std::string> source_map_;
};

} // namespace extensions

#endif // EXO_SHELL_RENDERER_EXTENSIONS_LOCAL_SOURCE_MAP_H_

