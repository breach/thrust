// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_BROWSER_WRAP_H_
#define BREACH_BROWSER_NODE_API_BROWSER_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "breach/browser/node/api/object_wrap.h"

namespace node {
class ObjectWrap;
}

namespace breach {

class Browser;

class BrowserWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  static void CreateNewBrowser(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ShowDevTools(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void CloseDevTools(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Size(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Position(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void DeleteTask(Browser* browser);

  void CreateTask();
  void CreateCallback(v8::Persistent<v8::Function>* pcb);

  void CloseTask();
  void ShowDevToolsTask();
  void CloseDevToolsTask();

  gfx::Size SizeGetter();
  gfx::Point PositionGetter();

  BrowserWrap();
  ~BrowserWrap();

  Browser*                             browser_;

  static v8::Persistent<v8::Function>  constructor;

  friend class base::RefCountedThreadSafe<BrowserWrap>;

  DISALLOW_COPY_AND_ASSIGN(BrowserWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_BROWSER_WRAP_H_

