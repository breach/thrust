// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/net/network_delegate.h"

#include "net/base/net_errors.h"
#include "net/base/static_cookie_policy.h"
#include "net/url_request/url_request.h"

using namespace content;

namespace exo_browser {

namespace {
bool g_accept_all_cookies = true;
}

ExoBrowserNetworkDelegate::ExoBrowserNetworkDelegate() 
{
}

ExoBrowserNetworkDelegate::~ExoBrowserNetworkDelegate() 
{
}

void 
ExoBrowserNetworkDelegate::SetAcceptAllCookies(
    bool accept) 
{
  g_accept_all_cookies = accept;
}

int 
ExoBrowserNetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  return net::OK;
}

int 
ExoBrowserNetworkDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) 
{
  return net::OK;
}

void 
ExoBrowserNetworkDelegate::OnSendHeaders(
    net::URLRequest* request,
    const net::HttpRequestHeaders& headers) 
{
}

int 
ExoBrowserNetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers) 
{
  return net::OK;
}

void 
ExoBrowserNetworkDelegate::OnBeforeRedirect(
    net::URLRequest* request,
    const GURL& new_location) 
{
}

void 
ExoBrowserNetworkDelegate::OnResponseStarted(
    net::URLRequest* request) 
{
}

void 
ExoBrowserNetworkDelegate::OnRawBytesRead(
    const net::URLRequest& request,
    int bytes_read) 
{
}

void 
ExoBrowserNetworkDelegate::OnCompleted(
    net::URLRequest* request, 
    bool started) 
{
}

void 
ExoBrowserNetworkDelegate::OnURLRequestDestroyed(
    net::URLRequest* request) 
{
}

void 
ExoBrowserNetworkDelegate::OnPACScriptError(
    int line_number,
    const string16& error) 
{
}

ExoBrowserNetworkDelegate::AuthRequiredResponse 
ExoBrowserNetworkDelegate::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials) 
{
  return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool 
ExoBrowserNetworkDelegate::OnCanGetCookies(
    const net::URLRequest& request,
    const net::CookieList& cookie_list) 
{
  net::StaticCookiePolicy::Type policy_type = g_accept_all_cookies ?
      net::StaticCookiePolicy::ALLOW_ALL_COOKIES :
      net::StaticCookiePolicy::BLOCK_SETTING_THIRD_PARTY_COOKIES;
  net::StaticCookiePolicy policy(policy_type);
  int rv = policy.CanGetCookies(
      request.url(), request.first_party_for_cookies());
  return rv == net::OK;
}

bool 
ExoBrowserNetworkDelegate::OnCanSetCookie(
    const net::URLRequest& request,
    const std::string& cookie_line,
    net::CookieOptions* options) 
{
  net::StaticCookiePolicy::Type policy_type = g_accept_all_cookies ?
      net::StaticCookiePolicy::ALLOW_ALL_COOKIES :
      net::StaticCookiePolicy::BLOCK_SETTING_THIRD_PARTY_COOKIES;
  net::StaticCookiePolicy policy(policy_type);
  int rv = policy.CanSetCookie(
      request.url(), request.first_party_for_cookies());
  return rv == net::OK;
}

bool 
ExoBrowserNetworkDelegate::OnCanAccessFile(
    const net::URLRequest& request,
    const base::FilePath& path) const 
{
  return true;
}

bool 
ExoBrowserNetworkDelegate::OnCanThrottleRequest(
    const net::URLRequest& request) const 
{
  return false;
}

int 
ExoBrowserNetworkDelegate::OnBeforeSocketStreamConnect(
    net::SocketStream* socket,
    const net::CompletionCallback& callback) 
{
  return net::OK;
}

void 
ExoBrowserNetworkDelegate::OnRequestWaitStateChange(
    const net::URLRequest& request,
    RequestWaitState waiting) 
{
}

} // namespace exo_browser
