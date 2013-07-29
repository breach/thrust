// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/geolocation/breach_access_token_store.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"

using namespace content;

namespace breach {

BreachAccessTokenStore::BreachAccessTokenStore(
    BrowserContext* browser_context)
  : browser_context_(browser_context),
    system_request_context_(NULL) 
  {
}

BreachAccessTokenStore::~BreachAccessTokenStore() {
}

void 
BreachAccessTokenStore::LoadAccessTokens(
    const LoadAccessTokensCallbackType& callback) 
{
  BrowserThread::PostTaskAndReply(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(&BreachAccessTokenStore::GetRequestContextOnUIThread,
                 this,
                 browser_context_),
      base::Bind(&BreachAccessTokenStore::RespondOnOriginatingThread,
                 this,
                 callback));
}

void 
BreachAccessTokenStore::GetRequestContextOnUIThread(
    BrowserContext* browser_context) 
{
  system_request_context_ = browser_context->GetRequestContext();
}

void 
BreachAccessTokenStore::RespondOnOriginatingThread(
    const LoadAccessTokensCallbackType& callback) 
{
  // Since content_shell is a test executable, rather than an end user program,
  // we provide a dummy access_token set to avoid hitting the server.
  AccessTokenSet access_token_set;
  access_token_set[GURL()] = ASCIIToUTF16("breach");
  callback.Run(access_token_set, system_request_context_);
}

void 
BreachAccessTokenStore::SaveAccessToken(
    const GURL& server_url, const string16& access_token) 
{
}

} // namespace breach
