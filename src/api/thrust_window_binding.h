// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_API_THRUST_SHELL_BINDING_H_
#define THRUST_SHELL_API_THRUST_SHELL_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace thrust_shell {

class ThrustWindow;

class ThrustWindowBinding : public APIBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ThrustWindowBinding(const unsigned int id, 
                  scoped_ptr<base::DictionaryValue> args);
  ~ThrustWindowBinding();

  virtual void CallLocalMethod(
      const std::string& method, 
      scoped_ptr<base::DictionaryValue> args, 
      const API::MethodCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  ThrustWindow* GetWindow();

private:
  scoped_ptr<ThrustWindow> window_;
};


// ## ThrustWindowBindingFactory
//
// Factory object used to generate ThrustWindow bindings
class ThrustWindowBindingFactory : public APIBindingFactory {
public:
  ThrustWindowBindingFactory();
  ~ThrustWindowBindingFactory();

  APIBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace thrust_shell
  
#endif // THRUST_SHELL_API_API_BINDING_H_
