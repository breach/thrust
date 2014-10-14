// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef EXO_SHELL_API_EXO_SESSION_BINDING_H_
#define EXO_SHELL_API_EXO_SESSION_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace exo_shell {

class ExoSession;

class ExoSessionBinding : public APIBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ExoSessionBinding(const unsigned int id, 
                    scoped_ptr<base::DictionaryValue> args);
  ~ExoSessionBinding();

  virtual void CallLocalMethod(
      const std::string& method, 
      scoped_ptr<base::DictionaryValue> args, 
      const API::MethodCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  ExoSession* GetSession();

private:
  scoped_ptr<ExoSession> session_;
};


// ## ExoSessionBindingFactory
//
// Factory object used to generate ExoSession bindings
class ExoSessionBindingFactory : public APIBindingFactory {
public:
  ExoSessionBindingFactory();
  ~ExoSessionBindingFactory();

  APIBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace exo_shell
  
#endif // EXO_SHELL_API_EXO_SESSION_BINDING_H_
