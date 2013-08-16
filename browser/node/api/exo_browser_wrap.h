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
  virtual ~ExoBrowserWrap();

  static void CreateExoBrowser(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);


  void CreateTask(const gfx::Size& size,
                  v8::Persistent<v8::Object>* browser_p,
                  v8::Persistent<v8::Function>* cb_p);
  void CreateCallback(v8::Persistent<v8::Object>* browser_p,
                      v8::Persistent<v8::Function>* cb_p);


  static void DeleteTask(ExoBrowser* browser);

  /****************************************************************************/
  /*                             WRAPPERS, TASKS                              */
  /****************************************************************************/
  static void Kill(const v8::FunctionCallbackInfo<v8::Value>& args);
  void KillTask(v8::Persistent<v8::Function>* cb_p);


  static void Size(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SizeTask(gfx::Size* size, 
                v8::Persistent<v8::Function>* cb_p);

  static void Position(const v8::FunctionCallbackInfo<v8::Value>& args);
  void PositionTask(gfx::Point* position,
                    v8::Persistent<v8::Function>* cb_p);


  static void SetControl(const v8::FunctionCallbackInfo<v8::Value>& args);
  /* TODO(spolu): Fix usage of (void*) */
  void SetControlTask(ExoBrowser::CONTROL_TYPE type, 
                      void* frame_w,
                      v8::Persistent<v8::Function>* cb_p);

  static void UnsetControl(const v8::FunctionCallbackInfo<v8::Value>& args);
  void UnsetControlTask(ExoBrowser::CONTROL_TYPE type,
                        v8::Persistent<v8::Function>* cb_p);

  static void SetControlDimension(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void SetControlDimensionTask(ExoBrowser::CONTROL_TYPE type, 
                               int size,
                               v8::Persistent<v8::Function>* cb_p);


  static void AddPage(const v8::FunctionCallbackInfo<v8::Value>& args);
  /* TODO(spolu): Fix usage of (void*) */
  void AddPageTask(void* frame_w,
                   v8::Persistent<v8::Function>* cb_p);

  static void RemovePage(const v8::FunctionCallbackInfo<v8::Value>& args);
  void RemovePageTask(const std::string& name,
                      v8::Persistent<v8::Function>* cb_p);

  static void ShowPage(const v8::FunctionCallbackInfo<v8::Value>& args);
  void ShowPageTask(const std::string& name,
                    v8::Persistent<v8::Function>* cb_p);

  /****************************************************************************/
  /*                              DISPATCHERS                                 */
  /****************************************************************************/
  static void SetOpenURLCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchOpenURL(const std::string& url, const std::string& from_frame);

  static void SetResizeCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchResize(const gfx::Size& size);

  static void SetKillCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchKill();

  static void SetFrameCloseCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameClose(const std::string& frame);

  static void SetFrameCreatedCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameCreated(const ExoFrame* frame);

  static void SetFrameKeyboardCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameKeyboard(const std::string& frame,
                             const content::NativeWebKeyboardEvent& event);

  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  ExoBrowser*                  browser_;

  v8::Persistent<v8::Function> open_url_cb_;
  v8::Persistent<v8::Function> resize_cb_;
  v8::Persistent<v8::Function> kill_cb_;
  v8::Persistent<v8::Function> frame_loading_state_change_cb_;
  v8::Persistent<v8::Function> frame_close_cb_;
  v8::Persistent<v8::Function> frame_navigate_cb_;
  v8::Persistent<v8::Function> frame_created_cb_;
  v8::Persistent<v8::Function> frame_keyboard_cb_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoBrowserWrap>;
  friend class ExoBrowser;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserWrap);
};

} // namespace breach

#endif // BREACH_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_

