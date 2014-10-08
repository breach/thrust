// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef EXO_SHELL_API_EXO_SHELL_BINDING_H_
#define EXO_SHELL_API_EXO_SHELL_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace exo_shell {

class ExoShell;

class ExoShellBinding : public APIBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ExoShellBinding(const unsigned int id, 
                  scoped_ptr<base::DictionaryValue> args);
  ~ExoShellBinding();

  virtual void CallLocalMethod(
      const std::string& method, 
      scoped_ptr<base::DictionaryValue> args, 
      const API::MethodCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/

private:
  scoped_ptr<ExoShell> shell_;
};


// ## ExoShellBindingFactory
//
// Factory object used to generate ExoShell bindings
class ExoShellBindingFactory : public APIBindingFactory {
public:
  ExoShellBindingFactory();
  ~ExoShellBindingFactory();

  APIBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace exo_shell
  
#endif // EXO_SHELL_API_API_BINDING_H_
