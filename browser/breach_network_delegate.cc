// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/shell_network_delegate.h"

#include "net/base/net_errors.h"
#include "net/base/static_cookie_policy.h"
#include "net/url_request/url_request.h"

using namespace content;

namespace breach {

namespace {
bool g_accept_all_cookies = true;
}

BreachNetworkDelegate::BreachNetworkDelegate() 
{
}

BreachNetworkDelegate::~BreachNetworkDelegate() 
{
}

void 
BreachNetworkDelegate::SetAcceptAllCookies(
    bool accept) 
{
  g_accept_all_cookies = accept;
}

int 
BreachNetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  return net::OK;
}

int 
BreachNetworkDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) 
{
  return net::OK;
}

void 
BreachNetworkDelegate::OnSendHeaders(
    net::URLRequest* request,
    const net::HttpRequestHeaders& headers) 
{
}

int 
BreachNetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers) 
{
  return net::OK;
}

void 
BreachNetworkDelegate::OnBeforeRedirect(
    net::URLRequest* request,
    const GURL& new_location) 
{
}

void 
BreachNetworkDelegate::OnResponseStarted(
    net::URLRequest* request) 
{
}

void 
BreachNetworkDelegate::OnRawBytesRead(
    const net::URLRequest& request,
    int bytes_read) 
{
}

void 
BreachNetworkDelegate::OnCompleted(
    net::URLRequest* request, 
    bool started) 
{
}

void 
BreachNetworkDelegate::OnURLRequestDestroyed(
    net::URLRequest* request) 
{
}

void 
BreachNetworkDelegate::OnPACScriptError(
    int line_number,
    const string16& error) 
{
}

BreachNetworkDelegate::AuthRequiredResponse 
BreachNetworkDelegate::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials) 
{
  return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool 
BreachNetworkDelegate::OnCanGetCookies(
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
BreachNetworkDelegate::OnCanSetCookie(
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
BreachNetworkDelegate::OnCanAccessFile(
    const net::URLRequest& request,
    const base::FilePath& path) const 
{
  return true;
}

bool 
BreachNetworkDelegate::OnCanThrottleRequest(
    const net::URLRequest& request) const 
{
  return false;
}

int 
BreachNetworkDelegate::OnBeforeSocketStreamConnect(
    net::SocketStream* socket,
    const net::CompletionCallback& callback) 
{
  return net::OK;
}

void 
BreachNetworkDelegate::OnRequestWaitStateChange(
    const net::URLRequest& request,
    RequestWaitState waiting) 
{
}

}  // namespace content
