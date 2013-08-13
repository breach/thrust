// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_
#define BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "breach/browser/node/api/object_wrap.h"
#include "breach/browser/node/api/exo_browser_wrap.h"
#include "breach/browser/node/node_thread.h"

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

  static void CreateExoFrame(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void CreateTask(const std::string& name,
                  const std::string& url,
                  v8::Persistent<v8::Object>* browser_p,
                  v8::Persistent<v8::Function>* cb_p);
  void CreateCallback(v8::Persistent<v8::Object>* frame_p,
                      v8::Persistent<v8::Function>* cb_p);

  static void DeleteTask(ExoFrame* frame);

  /****************************************************************************/
  /*                           WRAPPERS, TASKS                                */
  /****************************************************************************/
  static void LoadURL(const v8::FunctionCallbackInfo<v8::Value>& args);
  void LoadURLTask(const std::string& url,
                   v8::Persistent<v8::Function>* cb_p);

  static void GoBackOrForward(const v8::FunctionCallbackInfo<v8::Value>& args);
  void GoBackOrForwardTask(int offset,
                           v8::Persistent<v8::Function>* cb_p);

  static void Reload(const v8::FunctionCallbackInfo<v8::Value>& args);
  void ReloadTask(v8::Persistent<v8::Function>* cb_p);

  static void Stop(const v8::FunctionCallbackInfo<v8::Value>& args);
  void StopTask(v8::Persistent<v8::Function>* cb_p);

  static void Focus(const v8::FunctionCallbackInfo<v8::Value>& args);
  void FocusTask(v8::Persistent<v8::Function>* cb_p);


  static void Name(const v8::FunctionCallbackInfo<v8::Value>& args);
  void NameTask(std::string* name,
                v8::Persistent<v8::Function>* cb_p);

  static void Type(const v8::FunctionCallbackInfo<v8::Value>& args);
  void TypeTask(int* type,
                v8::Persistent<v8::Function>* cb_p);

  /****************************************************************************/
  /*                              DISPATCHERS                                 */
  /****************************************************************************/
  static void SetTitleUpdatedCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchTitleUpdated(const std::string& title);

  
  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  ExoFrame*                    frame_;

  v8::Persistent<v8::Function> title_updated_cb_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoFrameWrap>;
  friend class ExoBrowserWrap;
  friend class ExoFrame;

  DISALLOW_COPY_AND_ASSIGN(ExoFrameWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

