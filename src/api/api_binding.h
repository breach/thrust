// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_API_API_BINDING_H_
#define EXO_SHELL_API_API_BINDING_H_

#include <map>

#include "base/memory/scoped_ptr.h"
#include "base/values.h"

#include "src/api/api_handler.h"

namespace base {
class Value;
class DictionaryValue;
}

namespace exo_shell {

// ## ApiBinding
//
// Exposes the interface of an object binded to the API
class ApiBinding {
public:
  /****************************************************************************/
  /* VIRTUAL INTERFACE */
  /****************************************************************************/
  virtual void LocalCall(const std::string& method, 
                         scoped_ptr<base::DictionaryValue> args, 
                         const ApiHandler::ActionCallback& callback) = 0;

  virtual ~ApiBinding();

protected:
  /****************************************************************************/
  /* PROTECTED INTERFACE */
  /****************************************************************************/
  void RemoteCall(const std::string& method, 
                  scoped_ptr<base::DictionaryValue> args, 
                  const ApiHandler::ActionCallback& callback);
  void Emit(const std::string& type, 
            scoped_ptr<base::DictionaryValue> event);

  ApiBinding(const std::string& type, 
             const unsigned int id);

private:

  std::string      type_;
  unsigned int     id_;

  DISALLOW_COPY_AND_ASSIGN(ApiBinding);
};


// ## ApiBindingFactory
//
// Factory object used to generate typed binding objects
class ApiBindingFactory {
public:
  virtual ~ApiBindingFactory() {}

  virtual ApiBinding* Create(const unsigned int id, 
                             scoped_ptr<base::DictionaryValue> args) = 0;
};



} // namespace exo_shell
  
#endif // EXO_SHELL_API_API_BINDING_H_
