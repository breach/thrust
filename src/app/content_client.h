// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_APP_CONTENT_CLIENT_H_
#define EXO_SHELL_APP_CONTENT_CLIENT_H_

#include <string>
#include <vector>

#include "brightray/common/content_client.h"

namespace exo_shell {

class ContentClient : public brightray::ContentClient {
 public:
  ContentClient();
  virtual ~ContentClient();

 protected:
  /****************************************************************************/
  /* CONTENT_CLIENT IMPLEMENTATION */
  /****************************************************************************/
  virtual std::string GetProduct() const OVERRIDE;
  virtual void AddAdditionalSchemes(
      std::vector<std::string>* standard_schemes,
      std::vector<std::string>* savable_schemes) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ContentClient);
};

}  // namespace exo_shell

#endif // EXO_SHELL_APP_CONTENT_CLIENT_H_
