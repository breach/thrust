// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_BROWSER_WRAP_H_
#define BREACH_BROWSER_NODE_API_BROWSER_WRAP_H_

#include "v8/include/v8.h"
#include "third_party/node/src/node.h"

namespace node {
class ObjectWrap;
}

namespace breach {

class BrowserWrap : public node::ObjectWrap {
  public:
    static void Init(v8::Handle<v8::Object> exports);

  private:
    static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

    BrowserWrap();
    ~BrowserWrap();
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_BROWSER_WRAP_H_

