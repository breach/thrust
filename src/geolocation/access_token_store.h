// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_GEOLOCATION_ACCESS_TOKEN_STORE_H_
#define EXO_SHELL_GEOLOCATION_ACCESS_TOKEN_STORE_H_

#include "base/memory/ref_counted.h"
#include "content/public/browser/access_token_store.h"
#include "net/url_request/url_request_context_getter.h"

namespace exo_shell {

// Dummy access token store used to initialise the network location provider.
class ExoShellAccessTokenStore : public content::AccessTokenStore {
 public:
  explicit ExoShellAccessTokenStore();

  /****************************************************************************/
  /* ACCESSTOKENSTORE IMPLEMENTATION                                          */
  /****************************************************************************/
  virtual void LoadAccessTokens(
      const LoadAccessTokensCallbackType& callback) OVERRIDE;

  virtual void SaveAccessToken(
      const GURL& server_url, const base::string16& access_token) OVERRIDE;

 private:

  virtual ~ExoShellAccessTokenStore();

  DISALLOW_COPY_AND_ASSIGN(ExoShellAccessTokenStore);
};

}  // namespace exo_shell

#endif // EXO_SHELL_GEOLOCATION_ACCESS_TOKEN_STORE_H_
