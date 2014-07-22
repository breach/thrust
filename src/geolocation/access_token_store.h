// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_GEOLOCATION_ACCESS_TOKEN_STORE_H_
#define EXO_BROWSER_GEOLOCATION_ACCESS_TOKEN_STORE_H_

#include "base/memory/ref_counted.h"
#include "content/public/browser/access_token_store.h"
#include "net/url_request/url_request_context_getter.h"

namespace exo_browser {

// Dummy access token store used to initialise the network location provider.
class ExoBrowserAccessTokenStore : public content::AccessTokenStore {
 public:
  explicit ExoBrowserAccessTokenStore();

  /****************************************************************************/
  /* ACCESSTOKENSTORE IMPLEMENTATION                                          */
  /****************************************************************************/
  virtual void LoadAccessTokens(
      const LoadAccessTokensCallbackType& callback) OVERRIDE;

  virtual void SaveAccessToken(
      const GURL& server_url, const base::string16& access_token) OVERRIDE;

 private:
  /****************************************************************************/
  /* PRIVATE I/F                                                              */
  /****************************************************************************/
  void GetRequestContextOnUIThread();
  void RespondOnOriginatingThread(const LoadAccessTokensCallbackType& callback);

  virtual ~ExoBrowserAccessTokenStore();

  scoped_refptr<net::URLRequestContextGetter> system_request_context_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserAccessTokenStore);
};

}  // namespace exo_browser

#endif // EXO_BROWSER_GEOLOCATION_ACCESS_TOKEN_STORE_H_
