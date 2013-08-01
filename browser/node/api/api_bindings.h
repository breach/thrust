// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_API_EXTENION_H_
#define BREACH_BROWSER_NODE_API_API_EXTENION_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ui/base/resource/resource_bundle.h"
#include "v8/include/v8.h"

namespace breach {

class ApiBindings : public v8::Extension {
  public:
    ApiBindings();
    virtual ~ApiBindings();

    virtual v8::Handle<v8::FunctionTemplate> 
      GetNativeFunction(v8::Handle<v8::String> name) OVERRIDE;

  private:
    // Exposes the Breach API
    static void RequireBreach(
        const v8::FunctionCallbackInfo<v8::Value>& args);

  DISALLOW_COPY_AND_ASSIGN(ApiBindings);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_API_BINDINGS_H_
