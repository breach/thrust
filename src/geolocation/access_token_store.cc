// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/geolocation/access_token_store.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"

using namespace content;

namespace exo_browser {

ExoBrowserAccessTokenStore::ExoBrowserAccessTokenStore(
    BrowserContext* browser_context)
  : browser_context_(browser_context),
    system_request_context_(NULL) 
  {
}

ExoBrowserAccessTokenStore::~ExoBrowserAccessTokenStore() {
}

void 
ExoBrowserAccessTokenStore::LoadAccessTokens(
    const LoadAccessTokensCallbackType& callback) 
{
  BrowserThread::PostTaskAndReply(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(&ExoBrowserAccessTokenStore::GetRequestContextOnUIThread,
                 this,
                 browser_context_),
      base::Bind(&ExoBrowserAccessTokenStore::RespondOnOriginatingThread,
                 this,
                 callback));
}

void 
ExoBrowserAccessTokenStore::GetRequestContextOnUIThread(
    BrowserContext* browser_context) 
{
  system_request_context_ = browser_context->GetRequestContext();
}

void 
ExoBrowserAccessTokenStore::RespondOnOriginatingThread(
    const LoadAccessTokensCallbackType& callback) 
{
  // Since content_shell is a test executable, rather than an end user program,
  // we provide a dummy access_token set to avoid hitting the server.
  AccessTokenSet access_token_set;
  access_token_set[GURL()] = ASCIIToUTF16("exo_browser");
  callback.Run(access_token_set, system_request_context_);
}

void 
ExoBrowserAccessTokenStore::SaveAccessToken(
    const GURL& server_url, const string16& access_token) 
{
}

} // namespace exo_browser
