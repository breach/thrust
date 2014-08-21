// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_API_API_H_
#define EXO_SHELL_API_API_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

#include "base/files/file_path.h"
#include "net/socket/stream_listen_socket.h"
#include "net/socket/socket_descriptor.h"
#include "net/socket/unix_domain_socket_posix.h"

namespace base {
class Thread;
class Value;
class DictionaryValue;
}

namespace exo_shell {

class APIBinding;
class APIBindingFactory;

class API {
public:
  typedef base::Callback<void(const std::string& error, 
                              scoped_ptr<base::Value> result)> MethodCallback;
  typedef base::Callback<void(scoped_ptr<base::Value> event)> EventCallback;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  API();
  virtual ~API();

  static API* Get();

  // ### InstallBinding
  //
  // Installs a new binding for the API
  void InstallBinding(const std::string& type,
                      APIBindingFactory* factory);

  // ### Create
  // 
  // Creates a new object and return its `_target` id
  int Create(const std::string type,
             scoped_ptr<base::DictionaryValue> args);

  // ### Delete
  // 
  // Deletes the object designated by the target passed
  void Delete(unsigned int target);

  // ### CallMethod
  //
  // Call the method specified on the objcet represented by the provided target
  // id. The result of the call is returnd asynchronously to the callback.
  void CallMethod(unsigned int target,
                  std::string method,
                  scoped_ptr<base::DictionaryValue> args,
                  const API::MethodCallback& callback);

  // ### SetRemote
  //
  // Sets the remote for a given biding represented by its target id. The remote
  // is owned by the API (deleted on object deletion) if not replaced by another
  // one.
  void SetRemote(unsigned int target,
                 APIBindingRemote* remote);

private:
  /****************************************************************************/
  /* PRIVATE INTERFACE */
  /****************************************************************************/

  unsigned int                              next_binding_id_;
  std::map<std::string, APIBindingFactory*> factories_;
  std::map<unsigned int, APIBinding*>       bindings_;
  std::map<unsigned int, APIBindingRemote*> remotes_;
};

} // namespace exo_shell

#endif // EXO_SHELL_API_API_H_
