// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_API_EXO_BROWSER_BINDING_H_
#define EXO_BROWSER_API_EXO_BROWSER_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"

namespace exo_browser {

class ExoBrowser;

class ExoBrowserBinding : public ApiBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ExoBrowserBinding(const unsigned int id, 
                    scoped_ptr<base::DictionaryValue> args);
  ~ExoBrowserBinding();

  virtual void LocalCall(const std::string& method, 
                         scoped_ptr<base::DictionaryValue> args, 
                         const ApiHandler::ActionCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/

private:
  scoped_ptr<ExoBrowser> browser_;
};


// ## ExoBrowserBindingFactory
//
// Factory object used to generate ExoBrowser bindings
class ExoBrowserBindingFactory : public ApiBindingFactory {
public:
  ExoBrowserBindingFactory();
  ~ExoBrowserBindingFactory();

  ApiBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace exo_browser
  
#endif // EXO_BROWSER_API_API_BINDING_H_
