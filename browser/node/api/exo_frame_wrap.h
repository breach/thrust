// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_
#define BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "breach/browser/node/api/object_wrap.h"
#include "breach/browser/node/api/exo_browser_wrap.h"

namespace node {
class ObjectWrap;
}

namespace breach {

class ExoFrame;

class ExoFrameWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  ExoFrameWrap();
  ~ExoFrameWrap();

  static void CreateNewExoFrame(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void CreateTask(std::string& name,
                  std::string& url,
                  gfx::Point& position,
                  gfx::Size& size,
                  int zIndex);
  void CreateCallback(v8::Persistent<v8::Function>* pcb);

  static void DeleteTask(ExoFrame* frame);

  /****************************************************************************/
  /*                      WRAPPERS, TASKS & CALLBACKS                         */
  /****************************************************************************/
  static void SetVisible(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SetVisibleTask();
  void SetVisibleCallback();

  static void SetSize(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SetSizeTask();
  void SetSizeCallback();

  static void SetPosition(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SetPositionTask();
  void SetPositionCallback();

  static void Size(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SizeTask();
  void SizeCallback();

  static void Position(const v8::FunctionCallbackInfo<v8::Value>& args);
  void PositionTask();
  void PositionCallback();

  static void LoadURL(const v8::FunctionCallbackInfo<v8::Value>& args);
  void LoadURLTask();
  void LoadURLCallback();

  static void GoBack(const v8::FunctionCallbackInfo<v8::Value>& args);
  void GoBackTask();
  void GoBackCallback();

  static void GoForward(const v8::FunctionCallbackInfo<v8::Value>& args);
  void GoForwardTask();
  void GoForwardCallback();

  static void Reload(const v8::FunctionCallbackInfo<v8::Value>& args);
  void ReloadTask();
  void ReloadCallback();

  static void Stop(const v8::FunctionCallbackInfo<v8::Value>& args);
  void StopTask();
  void StopCallback();


  static void Name(const v8::FunctionCallbackInfo<v8::Value>& args);
  void NameTask();
  void NameCallback();

  static void Parent(const v8::FunctionCallbackInfo<v8::Value>& args);
  void ParentTask();
  void ParentCallback();


  static void SetTitleUpdatedCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void TitleUpdatedCallback(const std::string& frame);

  
  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  ExoFrame*   frame_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoFrameWrap>;
  friend class ExoBrowserWrap;
  friend class ExoFrame;

  DISALLOW_COPY_AND_ASSIGN(ExoFrameWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

