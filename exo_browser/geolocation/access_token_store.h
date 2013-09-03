// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_GEOLOCATION_ACCESS_TOKEN_STORE_H_
#define EXO_BROWSER_GEOLOCATION_ACCESS_TOKEN_STORE_H_

#include "content/public/browser/access_token_store.h"
#include "content/public/browser/browser_context.h"

namespace exo_browser {

// Dummy access token store used to initialise the network location provider.
class ExoBrowserAccessTokenStore : public content::AccessTokenStore {
 public:
  explicit ExoBrowserAccessTokenStore(
      content::BrowserContext* browser_context);

 private:
  virtual ~ExoBrowserAccessTokenStore();

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

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserAccessTokenStore);
};

} // namespace exo_browser

#endif // EXO_BROWSER_GEOLOCATION_ACCESS_TOKEN_STORE_H_
