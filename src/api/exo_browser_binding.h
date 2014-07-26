// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_API_EXO_BROWSER_BINDING_H_
#define EXO_BROWSER_API_EXO_BROWSER_BINDING_H_

namespace exo_browser {

class ExoBrowserBinding : ApiBinding {
public:
  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ExoBrowserBinding();
  ~ExoBrowserBinding();

  void LocalCall(std::string& method, 
                 Value* args, const &MethodCallback callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/

private
  ExoBrowser*   browser_;
};

} // namespace exo_browser
  
#endif // EXO_BROWSER_API_API_BINDING_H_
