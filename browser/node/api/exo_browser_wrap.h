// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_
#define BREACH_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "breach/browser/node/api/object_wrap.h"
#include "breach/browser/ui/exo_browser.h"

namespace node {
class ObjectWrap;
}

namespace breach {

class ExoBrowserWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  ExoBrowserWrap();
  ~ExoBrowserWrap();

  static void CreateNewExoBrowser(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);


  void CreateTask();
  void CreateCallback(v8::Persistent<v8::Function>* pcb);


  static void DeleteTask(ExoBrowser* browser);

  /****************************************************************************/
  /*                             WRAPPERS, TASKS                              */
  /****************************************************************************/
  static void Kill(const v8::FunctionCallbackInfo<v8::Value>& args);
  void KillTask();


  static void Size(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SizeTask(gfx::Size* size);

  static void Position(const v8::FunctionCallbackInfo<v8::Value>& args);
  void PositionTask(gfx::Point* position);


  static void SetControl(const v8::FunctionCallbackInfo<v8::Value>& args);
  /* TODO(spolu): Fix usage of (void*) */
  void SetControlTask(ExoBrowser::CONTROL_TYPE type, void* frame_w);

  static void UnsetControl(const v8::FunctionCallbackInfo<v8::Value>& args);
  void UnsetControlTask(ExoBrowser::CONTROL_TYPE type);

  static void SetControlDimension(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void SetControlDimensionTask(ExoBrowser::CONTROL_TYPE type, int size);


  static void AddPage(const v8::FunctionCallbackInfo<v8::Value>& args);
  /* TODO(spolu): Fix usage of (void*) */
  void AddPageTask(void* frame_w);

  static void RemovePage(const v8::FunctionCallbackInfo<v8::Value>& args);
  void RemovePageTask(const std::string& name);

  static void ShowPage(const v8::FunctionCallbackInfo<v8::Value>& args);
  void ShowPageTask(const std::string& name);

  /****************************************************************************/
  /*                              DISPATCHERS                                 */
  /****************************************************************************/
  static void SetOpenURLCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchOpenURL(const std::string& url, const std::string& from_frame);

  static void SetResizeCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchResize(const gfx::Size& size);

  static void SetFrameLoadingStateChangeCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameLoadingStateChange(const std::string& frame, bool loading);

  static void SetFrameCloseCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameClose(const std::string& frame);

  static void SetFrameNavigateCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameNavigate(const std::string& frame, const std::string& url);

  static void SetFrameCreatedCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameCreated(const ExoFrame* frame);

  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  ExoBrowser*                  browser_;

  v8::Persistent<v8::Function> open_url_cb_;
  v8::Persistent<v8::Function> resize_cb_;
  v8::Persistent<v8::Function> frame_loading_state_change_cb_;
  v8::Persistent<v8::Function> frame_close_cb_;
  v8::Persistent<v8::Function> frame_navigate_cb_;
  v8::Persistent<v8::Function> frame_created_cb_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoBrowserWrap>;
  friend class ExoBrowser;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_

