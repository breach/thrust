// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_API_API_H_
#define THRUST_SHELL_API_API_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace base {
class Thread;
class Value;
class DictionaryValue;
}

namespace thrust_shell {

class APIBinding;
class APIBindingFactory;
class APIBindingRemote;

// ## API
//
// This objects reprensents the ThrustShell API. It lives on the Browser UI thread
// and is used by object there as well as the APIServer.
class API {
public:
  typedef base::Callback<void(const std::string& error, 
                              scoped_ptr<base::DictionaryValue> result)> MethodCallback;
  typedef base::Callback<void(scoped_ptr<base::DictionaryValue> event)> EventCallback;

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
  // Sets the remote for a given biding represented by its target id. The Remote
  // is not owned by the API and should be nullified on deletion
  void SetRemote(unsigned int target,
                 APIBindingRemote* remote);

  // ### GetBinding
  //
  // Retrieves a binding based on its id
  APIBinding* GetBinding(unsigned int target);

  // ### GetRemote
  //
  // Retrieves a remote binding for this id
  APIBindingRemote* GetRemote(unsigned int target);

private:
  /****************************************************************************/
  /* PRIVATE INTERFACE */
  /****************************************************************************/

  static API*                               self_;
  unsigned int                              next_binding_id_;
  std::map<std::string, APIBindingFactory*>           factories_;
  std::map<unsigned int, scoped_refptr<APIBinding> >  bindings_;
  std::map<unsigned int, APIBindingRemote*>           remotes_;
};

} // namespace thrust_shell

#endif // THRUST_SHELL_API_API_H_
