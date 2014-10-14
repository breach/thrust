// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef EXO_SHELL_API_EXO_MENU_BINDING_H_
#define EXO_SHELL_API_EXO_MENU_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace exo_shell {

class ExoMenu;

class ExoMenuBinding : public APIBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ExoMenuBinding(const unsigned int id, 
                  scoped_ptr<base::DictionaryValue> args);
  ~ExoMenuBinding();

  virtual void CallLocalMethod(
      const std::string& method, 
      scoped_ptr<base::DictionaryValue> args, 
      const API::MethodCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  ExoMenu* GetMenu();

private:
  scoped_ptr<ExoMenu> menu_;
};


// ## ExoMenuBindingFactory
//
// Factory object used to generate ExoMenu bindings
class ExoMenuBindingFactory : public APIBindingFactory {
public:
  ExoMenuBindingFactory();
  ~ExoMenuBindingFactory();

  APIBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace exo_shell
  
#endif // EXO_SHELL_API_EXO_MENU_BINDING_H_
