// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_API_THRUST_SESSION_BINDING_H_
#define THRUST_SHELL_API_THRUST_SESSION_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace thrust_shell {

class ThrustSession;

class ThrustSessionBinding : public APIBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ThrustSessionBinding(const unsigned int id, 
                    scoped_ptr<base::DictionaryValue> args);
  ~ThrustSessionBinding();

  virtual void CallLocalMethod(
      const std::string& method, 
      scoped_ptr<base::DictionaryValue> args, 
      const API::MethodCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  ThrustSession* GetSession();

private:
  scoped_ptr<ThrustSession> session_;
};


// ## ThrustSessionBindingFactory
//
// Factory object used to generate ThrustSession bindings
class ThrustSessionBindingFactory : public APIBindingFactory {
public:
  ThrustSessionBindingFactory();
  ~ThrustSessionBindingFactory();

  APIBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace thrust_shell
  
#endif // THRUST_SHELL_API_THRUST_SESSION_BINDING_H_
