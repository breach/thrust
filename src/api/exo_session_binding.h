// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_API_EXO_SESSION_BINDING_H_
#define EXO_SHELL_API_EXO_SESSION_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace exo_shell {

class ExoSession;

class ExoSessionBinding : public ApiBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ExoSessionBinding(const unsigned int id, 
                    scoped_ptr<base::DictionaryValue> args);
  ~ExoSessionBinding();

  virtual void LocalCall(const std::string& method, 
                         scoped_ptr<base::DictionaryValue> args, 
                         const ApiHandler::ActionCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/

private:
  scoped_ptr<ExoSession> session_;
};


// ## ExoSessionBindingFactory
//
// Factory object used to generate ExoSession bindings
class ExoSessionBindingFactory : public ApiBindingFactory {
public:
  ExoSessionBindingFactory();
  ~ExoSessionBindingFactory();

  ApiBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace exo_shell
  
#endif // EXO_SHELL_API_EXO_SESSION_BINDING_H_
