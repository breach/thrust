// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_NODE_API_EXO_SESSION_WRAP_H_
#define EXO_BROWSER_NODE_API_EXO_SESSION_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "net/cookies/cookie_monster.h"
#include "exo_browser/src/node/api/object_wrap.h"

namespace node {
class ObjectWrap;
}

namespace exo_browser {

class ExoSession;

class ExoSessionWrap : public ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  ExoSessionWrap();
  virtual ~ExoSessionWrap();

  static void CreateExoSession(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);


  void CreateTask(bool off_the_record,
                  const std::string& path,
                  v8::Persistent<v8::Object>* session_p,
                  v8::Persistent<v8::Function>* cb_p);
  void CreateCallback(v8::Persistent<v8::Object>* session_p,
                      v8::Persistent<v8::Function>* cb_p);


  static void DeleteTask(ExoSession* session);

  /****************************************************************************/
  /* WRAPPERS, TASKS */
  /****************************************************************************/

  static void OffTheRecord(const v8::FunctionCallbackInfo<v8::Value>& args);
  void OffTheRecordTask(bool* off_the_record,
                        v8::Persistent<v8::Function>* cb_p);

  static void AddVisitedLink(const v8::FunctionCallbackInfo<v8::Value>& args);
  void AddVisitedLinkTask(const std::string& url,
                          v8::Persistent<v8::Function>* cb_p);
  static void ClearVisitedLinks(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void ClearVisitedLinksTask(v8::Persistent<v8::Function>* cb_p);

  static void GetDevToolsURL(const v8::FunctionCallbackInfo<v8::Value>& args);
  void GetDevToolsURLTask(std::string* url,
                          v8::Persistent<v8::Function>* cb_p);

  /****************************************************************************/
  /* HANDLERS */
  /****************************************************************************/
  typedef net::CookieMonster::PersistentCookieStore::LoadedCallback
    LoadedCallback;

  static void SetCookiesLoadForKeyHandler(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void CookiesLoadForKeyCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void CallCookiesLoadForKey(const std::string& key, const LoadedCallback& cb);
  void CallCookiesLoad(const LoadedCallback& cb);


  static void SetCookiesFlushHandler(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void CookiesFlushCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void CallCookiesFlush(const base::Closure& cb);


  /****************************************************************************/
  /* DISPATCHERS */
  /****************************************************************************/
  static void SetCookiesAddCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchCookiesAdd(const net::CanonicalCookie& cc);

  static void SetCookiesDeleteCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchCookiesDelete(const net::CanonicalCookie& cc);

  static void SetCookiesUpdateAccessTimeCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchCookiesUpdateAccessTime(const net::CanonicalCookie& cc);

  static void SetCookiesForceKeepSessionStateCallback(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  void DispatchCookiesForceKeepSessionState();

  /****************************************************************************/
  /* MEMBERS */
  /****************************************************************************/
  ExoSession*                  session_;

  int                           cookies_load_rid_;
  std::map<int, LoadedCallback> cookies_load_reqs_;
  v8::Persistent<v8::Function>  cookies_load_hdlr_;
  
  int                           cookies_flush_rid_;
  std::map<int, base::Closure>  cookies_flush_reqs_;
  v8::Persistent<v8::Function>  cookies_flush_hdlr_;

  v8::Persistent<v8::Function>  cookies_add_cb_;
  v8::Persistent<v8::Function>  cookies_delete_cb_;
  v8::Persistent<v8::Function>  cookies_update_access_time_cb_;
  v8::Persistent<v8::Function>  cookies_force_keep_session_state_cb_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoSessionWrap>;
  friend class ExoFrameWrap;
  friend class ExoSession;
  friend class ExoSessionCookieStore;

  DISALLOW_COPY_AND_ASSIGN(ExoSessionWrap);
};

} // namespace exo_browser

#endif // EXO_BROWSER_NODE_API_EXO_SESSION_WRAP_H_
