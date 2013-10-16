// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_NODE_API_EXO_FRAME_WRAP_H_
#define EXO_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "content/public/common/stop_find_action.h"
#include "exo_browser/src/node/api/object_wrap.h"
#include "exo_browser/src/node/api/exo_browser_wrap.h"
#include "exo_browser/src/node/node_thread.h"

namespace node {
class ObjectWrap;
}

namespace content {
struct FaviconURL;
}

namespace exo_browser {

class ExoFrame;

class ExoFrameWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  ExoFrameWrap();
  virtual ~ExoFrameWrap();

  static void CreateExoFrame(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  /* TODO(spolu): Fix usage of (void*) */
  void CreateTask(const std::string& name,
                  const std::string& url,
                  void* session_w,
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

  static void Undo(const v8::FunctionCallbackInfo<v8::Value>& args);
  void UndoTask(v8::Persistent<v8::Function>* cb_p);
  static void Redo(const v8::FunctionCallbackInfo<v8::Value>& args);
  void RedoTask(v8::Persistent<v8::Function>* cb_p);
  static void CutSelection(const v8::FunctionCallbackInfo<v8::Value>& args);
  void CutSelectionTask(v8::Persistent<v8::Function>* cb_p);
  static void CopySelection(const v8::FunctionCallbackInfo<v8::Value>& args);
  void CopySelectionTask(v8::Persistent<v8::Function>* cb_p);
  static void Paste(const v8::FunctionCallbackInfo<v8::Value>& args);
  void PasteTask(v8::Persistent<v8::Function>* cb_p);
  static void DeleteSelection(const v8::FunctionCallbackInfo<v8::Value>& args);
  void DeleteSelectionTask(v8::Persistent<v8::Function>* cb_p);
  static void SelectAll(const v8::FunctionCallbackInfo<v8::Value>& args);
  void SelectAllTask(v8::Persistent<v8::Function>* cb_p);
  static void Unselect(const v8::FunctionCallbackInfo<v8::Value>& args);
  void UnselectTask(v8::Persistent<v8::Function>* cb_p);

  static void Focus(const v8::FunctionCallbackInfo<v8::Value>& args);
  void FocusTask(v8::Persistent<v8::Function>* cb_p);

  static void Find(const v8::FunctionCallbackInfo<v8::Value>& args);
  void FindTask(int request_id, const std::string& search_text,
                bool forward, bool matchCase, bool findNext,
                v8::Persistent<v8::Function>* cb_p);

  static void StopFinding(const v8::FunctionCallbackInfo<v8::Value>& args);
  void StopFindingTask(content::StopFindAction action,
                       v8::Persistent<v8::Function>* cb_p);


  static void Name(const v8::FunctionCallbackInfo<v8::Value>& args);
  void NameTask(std::string* name,
                v8::Persistent<v8::Function>* cb_p);

  static void Type(const v8::FunctionCallbackInfo<v8::Value>& args);
  void TypeTask(int* type,
                v8::Persistent<v8::Function>* cb_p);

  /****************************************************************************/
  /*                              DISPATCHERS                                 */
  /****************************************************************************/
  static void SetLoadFailCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchLoadFail(const std::string& url,
                        const int error_code,
                        const std::string& error_desc);

  static void SetLoadFinishCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchLoadFinish(const std::string& url);


  static void SetLoadingStartCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchLoadingStart();

  static void SetLoadingStopCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchLoadingStop();

  static void SetFaviconUpdateCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchFaviconUpdate(
      const std::vector<content::FaviconURL>& candidates);

  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  ExoFrame*                    frame_;

  v8::Persistent<v8::Function> title_update_cb_;
  v8::Persistent<v8::Function> favicon_update_cb_;

  v8::Persistent<v8::Function> pending_url_cb_;
  v8::Persistent<v8::Function> load_fail_cb_;
  v8::Persistent<v8::Function> load_finish_cb_;
  v8::Persistent<v8::Function> loading_start_cb_;
  v8::Persistent<v8::Function> loading_stop_cb_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoFrameWrap>;
  friend class ExoBrowserWrap;
  friend class ExoFrame;

  DISALLOW_COPY_AND_ASSIGN(ExoFrameWrap);
};

} // namespace exo_browser

#endif // EXO_BROWSER_NODE_API_EXO_FRAME_WRAP_H_

