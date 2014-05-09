// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_NODE_API_API_EXTENION_H_
#define EXO_BROWSER_NODE_API_API_EXTENION_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/base/resource/resource_bundle.h"
#include "v8/include/v8.h"

namespace exo_browser {

class ApiBindings : public v8::Extension {
  public:
    ApiBindings();
    virtual ~ApiBindings();

    virtual v8::Handle<v8::FunctionTemplate> 
      GetNativeFunctionTemplate(
          v8::Isolate *isolate,
          v8::Handle<v8::String> name) OVERRIDE;

  private:
    // Exposes the ExoBrowser API
    static void RequireExoBrowser(
        const v8::FunctionCallbackInfo<v8::Value>& args);

  DISALLOW_COPY_AND_ASSIGN(ApiBindings);
};

} // namespace exo_browser

#endif // EXO_BROWSER_NODE_API_API_BINDINGS_H_
