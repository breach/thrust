// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/geolocation/access_token_store.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"

#include "src/browser/session/thrust_session.h"
#include "src/browser/browser_client.h"

#ifndef GOOGLEAPIS_API_KEY
#define GOOGLEAPIS_API_KEY "AIzaSyAQfxPJiounkhOjODEO5ZieffeBv6yft2Q"
#endif

using namespace content;

namespace thrust_shell {

namespace {

// Notice that we just combined the api key with the url together here, because
// if we use the standard {url: key} format Chromium would override our key with
// the predefined one in common.gypi of libchromiumcontent, which is empty.
const char* kGeolocationProviderUrl =
    "https://www.googleapis.com/geolocation/v1/geolocate?key="
    GOOGLEAPIS_API_KEY;

}  // namespace

ThrustShellAccessTokenStore::ThrustShellAccessTokenStore()
{
}

ThrustShellAccessTokenStore::~ThrustShellAccessTokenStore() 
{
}

void 
ThrustShellAccessTokenStore::LoadAccessTokens(
    const LoadAccessTokensCallbackType& callback) 
{
  AccessTokenSet access_token_set;

  // Equivelent to access_token_set[kGeolocationProviderUrl].
  // Somehow base::string16 is causing compilation errors when used in a pair
  // of std::map on Linux, this can work around it.
  std::pair<GURL, base::string16> token_pair;
  token_pair.first = GURL(kGeolocationProviderUrl);
  access_token_set.insert(token_pair);

  callback.Run(access_token_set,
               ThrustShellBrowserClient::Get()->system_session()->url_request_context_getter());
}

void ThrustShellAccessTokenStore::SaveAccessToken(
    const GURL& server_url, const base::string16& access_token) {
  LOG(INFO) << "ExoBrwoserAccessTokenStore::SaveAccessToken: " 
            << server_url << " "
            << access_token;
}

}  // namespace content
