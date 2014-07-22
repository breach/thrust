// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/geolocation/access_token_store.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/shell/browser/shell_browser_context.h"

#include "src/browser/session/exo_session.h"
#include "src/browser/content_browser_client.h"


using namespace content;

namespace exo_browser {

ExoBrowserAccessTokenStore::ExoBrowserAccessTokenStore()
  : system_request_context_(NULL)
{
}

ExoBrowserAccessTokenStore::~ExoBrowserAccessTokenStore() 
{
}

void 
ExoBrowserAccessTokenStore::LoadAccessTokens(
    const LoadAccessTokensCallbackType& callback) 
{
  AccessTokenSet access_token_set;
  access_token_set[GURL()] = base::ASCIIToUTF16("exo_browser");
  callback.Run(access_token_set, system_request_context_.get());

  BrowserThread::PostTaskAndReply(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(&ExoBrowserAccessTokenStore::GetRequestContextOnUIThread,
                 this),
      base::Bind(&ExoBrowserAccessTokenStore::RespondOnOriginatingThread,
                 this,
                 callback));
}

void ExoBrowserAccessTokenStore::GetRequestContextOnUIThread() {
  system_request_context_ = 
    ExoBrowserContentBrowserClient::Get()->system_session()->GetRequestContext();
}

void ExoBrowserAccessTokenStore::RespondOnOriginatingThread(
    const LoadAccessTokensCallbackType& callback) {
  /* TODO(spolu): For now we provide a dummy value to prevent crash. We */
  /*              add proper tokens when relevant.                      */
  AccessTokenSet access_token_set;
  access_token_set[GURL()] = base::ASCIIToUTF16("exo_browser");
  callback.Run(access_token_set, system_request_context_.get());
  system_request_context_ = NULL;
}

void ExoBrowserAccessTokenStore::SaveAccessToken(
    const GURL& server_url, const base::string16& access_token) {
  LOG(INFO) << "ExoBrwoserAccessTokenStore::SaveAccessToken: " 
            << server_url << " "
            << access_token;
}

}  // namespace content
