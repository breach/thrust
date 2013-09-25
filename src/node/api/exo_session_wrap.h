// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_NODE_API_EXO_SESSION_WRAP_H_
#define EXO_BROWSER_NODE_API_EXO_SESSION_WRAP_H_

#include "base/memory/scoped_ptr.h"
#include "v8/include/v8.h"
#include "exo_browser/src/node/api/object_wrap.h"

namespace node {
class ObjectWrap;
}

namespace exo_browser {

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
  /*                             WRAPPERS, TASKS                              */
  /****************************************************************************/

  /****************************************************************************/
  /*                                HANDLERS                                  */
  /****************************************************************************/

  /****************************************************************************/
  /*                               DISPATCHERS                                */
  /****************************************************************************/

  /****************************************************************************/
  /*                                MEMBERS                                   */
  /****************************************************************************/
  ExoSession*                  session_;

  static v8::Persistent<v8::Function>  s_constructor;

  friend class base::RefCountedThreadSafe<ExoSessionWrap>;
  friend class ExoSession;

  DISALLOW_COPY_AND_ASSIGN(ExoSessionWrap);
};

} // namespace exo_browser

#endif // EXO_BROWSER_NODE_API_EXO_SESSION_WRAP_H_
