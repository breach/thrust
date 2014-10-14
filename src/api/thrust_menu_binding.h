// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_API_THRUST_MENU_BINDING_H_
#define THRUST_SHELL_API_THRUST_MENU_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace thrust_shell {

class ThrustMenu;

class ThrustMenuBinding : public APIBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ThrustMenuBinding(const unsigned int id, 
                  scoped_ptr<base::DictionaryValue> args);
  ~ThrustMenuBinding();

  virtual void CallLocalMethod(
      const std::string& method, 
      scoped_ptr<base::DictionaryValue> args, 
      const API::MethodCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  void EmitExecuteCommand(int command_id, int event_flags);

  ThrustMenu* GetMenu();

private:
  scoped_ptr<ThrustMenu> menu_;
};


// ## ThrustMenuBindingFactory
//
// Factory object used to generate ThrustMenu bindings
class ThrustMenuBindingFactory : public APIBindingFactory {
public:
  ThrustMenuBindingFactory();
  ~ThrustMenuBindingFactory();

  APIBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace thrust_shell
  
#endif // THRUST_SHELL_API_THRUST_MENU_BINDING_H_
