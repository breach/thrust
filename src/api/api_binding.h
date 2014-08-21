// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_API_API_BINDING_H_
#define EXO_SHELL_API_API_BINDING_H_

#include <map>

#include "base/memory/scoped_ptr.h"
#include "base/values.h"

#include "src/api/api.h"

namespace base {
class Value;
class DictionaryValue;
}

namespace exo_shell {

// ## APIBinding
//
// Exposes the interface of an object binded to the API
class APIBinding {
public:
  /****************************************************************************/
  /* VIRTUAL INTERFACE */
  /****************************************************************************/
  virtual void CallLocalMethod(const std::string& method, 
                               scoped_ptr<base::DictionaryValue> args, 
                               const API::ActionCallback& callback) = 0;

  virtual ~APIBinding();

protected:
  /****************************************************************************/
  /* PROTECTED INTERFACE */
  /****************************************************************************/
  void CallRemoteMethod(const std::string& method, 
                        scoped_ptr<base::DictionaryValue> args, 
                        const API::ActionCallback& callback);
  void Emit(const std::string& type, 
            scoped_ptr<base::DictionaryValue> event);

  APIBinding(const std::string& type, 
             const unsigned int id);

private:

  std::string      type_;
  unsigned int     id_;

  DISALLOW_COPY_AND_ASSIGN(APIBinding);
};


// ## APIBindingFactory
//
// Factory object used to generate typed binding objects
class APIBindingFactory {
public:
  virtual ~APIBindingFactory() {}

  virtual APIBinding* Create(const unsigned int id, 
                             scoped_ptr<base::DictionaryValue> args) = 0;
};

// ## APIBindingRemote
//
// APIBindingRemote Interface to call remote object methods and emit events.
// Remote implementations can be installed through the API::SetRemote method 
// and only one remote is associated with a given binding
class APIBindingRemote {
public:
  virtual ~APIBindingRemote() {}

  virtual void CallMethod(const std::string method,
                          scoped_ptr<base::DictionaryValue> args,
                          const API::MethodCallback& callback) = 0;
  virtual void EmitEvent(const std::string type,
                         scoped_ptr<base::DictionaryValue> event) = 0;
};



} // namespace exo_shell
  
#endif // EXO_SHELL_API_API_BINDING_H_
