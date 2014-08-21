// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_API_EXO_MENU_BINDING_H_
#define EXO_SHELL_API_EXO_MENU_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"
#include "ui/base/models/simple_menu_model.h"
namespace exo_shell {

class ExoMenu;

class ExoMenuBinding : public ApiBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ExoMenuBinding(const unsigned int id, 
                    scoped_ptr<base::DictionaryValue> args);
  ~ExoMenuBinding();

  virtual void LocalCall(const std::string& method, 
                         scoped_ptr<base::DictionaryValue> args, 
                         const ApiHandler::ActionCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/

private:
   scoped_ptr<ExoMenu> menu_;
};


// ## ExoShellBindingFactory
//
// Factory object used to generate ExoShell bindings
class ExoMenuBindingFactory : public ApiBindingFactory {
public:
  ExoMenuBindingFactory();
  ~ExoMenuBindingFactory();

  ApiBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace exo_shell
  
#endif // EXO_SHELL_API_API_BINDING_H_
