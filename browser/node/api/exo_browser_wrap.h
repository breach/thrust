// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_
#define BREACH_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "breach/browser/node/api/object_wrap.h"

namespace node {
class ObjectWrap;
}

namespace breach {

class ExoBrowser;

class ExoBrowserWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  ExoBrowserWrap(ExoBrowser* browser);
  ~ExoBrowserWrap();

  static void CreateNewExoBrowser(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void CreateTask();
  void CreateCallback(v8::Persistent<v8::Function>* pcb);


  static void DeleteTask(Browser* browser);

  /****************************************************************************/
  /*                      WRAPPERS, TASKS & CALLBACKS                         */
  /****************************************************************************/
  static void Kill(const v8::FunctionCallbackInfo<v8::Value>& args);
  void KillTask();
  void KillCallback();

  static void Size(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SizeTask();
  void SizeCallback();

  static void Position(const v8::FunctionCallbackInfo<v8::Value>& args);
  void PositionTask();
  void PositionCallback();

  static void SetOpenURLCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void OpenURLCallback(const GURL& url, const std::string& source_frame);

  static void SetResizeCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void ResizeCallback();

  //static void AddFrame(const v8::FunctionCallbackInfo<v8::Value>& args);

  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  ExoBrowser*                  browser_;

  v8::Persistent<v8::Function> load_url_cb_;
  v8::Persistent<v8::Function> resize_cb_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoBrowserWrap>;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_

