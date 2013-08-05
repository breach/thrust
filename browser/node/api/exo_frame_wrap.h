// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_
#define BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "breach/browser/node/api/object_wrap.h"

namespace node {
class ObjectWrap;
}

namespace breach {

class ExoFrame;

class ExoFrameWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  //static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetVisible(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetPosition(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void Size(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Position(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void LoadURL(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GoBack(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GoForward(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Reload(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Stop(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void Name(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Parent(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void DeleteTask(Browser* browser);


  void SetVisibleTask();
  void SetVisibleCallback();

  void SetSizeTask();
  void SetSizeCallback();

  void SizeTask();
  void SizeCallback();

  void PositionTask();
  void PositionCallback();
  
  void LoadURLTask();
  void LoadURLCallback();

  void GoBackTask();
  void GoBackCallback();

  void GoForwardTask();
  void GoForwardCallback();

  void ReloadTask();
  void ReloadCallback();

  void StopTask();
  void StopCallback();

  void NameTask();
  void NameCallback();

  void ParentTask();
  void ParentCallback();


  ExoFrameWrap();
  ~ExoFrameWrap();

  ExoFrame*   frame_;

  friend class base::RefCountedThreadSafe<ExoFrameWrap>;

  DISALLOW_COPY_AND_ASSIGN(ExoFrameWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

