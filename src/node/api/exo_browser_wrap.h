// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_
#define EXO_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"
#include "exo_browser/src/node/api/object_wrap.h"
#include "exo_browser/src/browser/ui/exo_browser.h"

namespace node {
class ObjectWrap;
}

namespace exo_browser {

class ExoSession;

class ExoBrowserWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

  // ### NavigationEntry
  // Structure used to pass navigation entry state to the Wrapper
  struct NavigationEntry {
    std::string url_;
    std::string virtual_url_;
    std::string title_;

    bool        visible_;
    int64       timestamp_;

    int         id_;
    std::string type_;
    std::string ssl_security_type_;
    uint32      ssl_cert_status_;
    int         ssl_content_status_;
  };

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

  static void Focus(const v8::FunctionCallbackInfo<v8::Value>& args);
  void FocusTask(v8::Persistent<v8::Function>* cb_p);


  static void Maximize(const v8::FunctionCallbackInfo<v8::Value>& args);
  void MaximizeTask(v8::Persistent<v8::Function>* cb_p);


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
  void DispatchOpenURL(const std::string& url, 
                       const WindowOpenDisposition disposition,
                       const std::string& from_frame);

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
  void DispatchFrameCreated(const std::string& src_frame,
                            const WindowOpenDisposition disposition,
                            const gfx::Rect& initial_pos,
                            ExoFrame* new_frame);
  /* TODO(spolu): Fix usage of (void*) */
  void FrameCreatedTask(const std::string& src_frame,
                        const WindowOpenDisposition disposition,
                        const gfx::Rect& initial_pos,
                        ExoFrame* new_frame,
                        void* frame_w,
                        v8::Persistent<v8::Object>* frame_p);
  void FrameCreatedFinish(const std::string& src_frame,
                          const WindowOpenDisposition disposition,
                          const gfx::Rect& initial_pos,
                          v8::Persistent<v8::Object>* frame_p);

  static void SetFrameKeyboardCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFrameKeyboard(const std::string& frame,
                             const content::NativeWebKeyboardEvent& event);

  static void SetNavigationStateCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchNavigationState(const std::string& frame,
                               const std::vector<NavigationEntry>& entries,
                               bool can_go_back,
                               bool can_go_forward);
                               

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
  v8::Persistent<v8::Function> navigation_state_cb_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoBrowserWrap>;
  friend class ExoBrowser;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserWrap);
};

} // namespace exo_browser

#endif // EXO_BROWSER_NODE_API_EXO_BROWSER_WRAP_H_

