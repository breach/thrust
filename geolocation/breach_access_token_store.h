// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_GEOLOCATION_BREACH_ACCESS_TOKEN_STORE_H_
#define BREACH_GEOLOCATION_BREACH_ACCESS_TOKEN_STORE_H_

#include "content/public/browser/access_token_store.h"
#include "content/public/browser/browser_context.h"

namespace breach {

// Dummy access token store used to initialise the network location provider.
class BreachAccessTokenStore : public content::AccessTokenStore {
 public:
  explicit BreachAccessTokenStore(
      content::BrowserContext* browser_context);

 private:
  virtual ~BreachAccessTokenStore();

  void GetRequestContextOnUIThread(
      content::BrowserContext* browser_context);
  void RespondOnOriginatingThread(
      const LoadAccessTokensCallbackType& callback);

  // AccessTokenStore
  virtual void LoadAccessTokens(
      const LoadAccessTokensCallbackType& callback) OVERRIDE;

  virtual void SaveAccessToken(
      const GURL& server_url, const string16& access_token) OVERRIDE;

  content::BrowserContext* browser_context_;
  net::URLRequestContextGetter* system_request_context_;

  DISALLOW_COPY_AND_ASSIGN(BreachAccessTokenStore);
};

} // namespace breach

#endif // BREACH_GEOLOCATION_BREACH_ACCESS_TOKEN_STORE_H_
