// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/net/network_delegate.h"

#include "net/base/net_errors.h"
#include "net/base/static_cookie_policy.h"
#include "net/url_request/url_request.h"

using namespace content;

namespace exo_shell {

namespace {
bool g_accept_all_cookies = true;
}

ExoShellNetworkDelegate::ExoShellNetworkDelegate() 
{
}

ExoShellNetworkDelegate::~ExoShellNetworkDelegate() 
{
}

void 
ExoShellNetworkDelegate::SetAcceptAllCookies(
    bool accept) 
{
  g_accept_all_cookies = accept;
}

int 
ExoShellNetworkDelegate::OnBeforeURLRequest(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    GURL* new_url) {
  return net::OK;
}

int 
ExoShellNetworkDelegate::OnBeforeSendHeaders(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    net::HttpRequestHeaders* headers) 
{
  return net::OK;
}

void 
ExoShellNetworkDelegate::OnSendHeaders(
    net::URLRequest* request,
    const net::HttpRequestHeaders& headers) 
{
}

int 
ExoShellNetworkDelegate::OnHeadersReceived(
    net::URLRequest* request,
    const net::CompletionCallback& callback,
    const net::HttpResponseHeaders* original_response_headers,
    scoped_refptr<net::HttpResponseHeaders>* override_response_headers,
    GURL* allowed_unsafe_redirect_url)
{
  return net::OK;
}

void 
ExoShellNetworkDelegate::OnBeforeRedirect(
    net::URLRequest* request,
    const GURL& new_location) 
{
}

void 
ExoShellNetworkDelegate::OnResponseStarted(
    net::URLRequest* request) 
{
}

void 
ExoShellNetworkDelegate::OnRawBytesRead(
    const net::URLRequest& request,
    int bytes_read) 
{
}

void 
ExoShellNetworkDelegate::OnCompleted(
    net::URLRequest* request, 
    bool started) 
{
}

void 
ExoShellNetworkDelegate::OnURLRequestDestroyed(
    net::URLRequest* request) 
{
}

void 
ExoShellNetworkDelegate::OnPACScriptError(
    int line_number,
    const base::string16& error) 
{
}

ExoShellNetworkDelegate::AuthRequiredResponse 
ExoShellNetworkDelegate::OnAuthRequired(
    net::URLRequest* request,
    const net::AuthChallengeInfo& auth_info,
    const AuthCallback& callback,
    net::AuthCredentials* credentials) 
{
  return AUTH_REQUIRED_RESPONSE_NO_ACTION;
}

bool 
ExoShellNetworkDelegate::OnCanGetCookies(
    const net::URLRequest& request,
    const net::CookieList& cookie_list) 
{
  return true;
  net::StaticCookiePolicy::Type policy_type = g_accept_all_cookies ?
      net::StaticCookiePolicy::ALLOW_ALL_COOKIES :
      net::StaticCookiePolicy::BLOCK_ALL_THIRD_PARTY_COOKIES;
  net::StaticCookiePolicy policy(policy_type);
  int rv = policy.CanGetCookies(
      request.url(), request.first_party_for_cookies());
  return rv == net::OK;
}

bool 
ExoShellNetworkDelegate::OnCanSetCookie(
    const net::URLRequest& request,
    const std::string& cookie_line,
    net::CookieOptions* options) 
{
  return true;
  net::StaticCookiePolicy::Type policy_type = g_accept_all_cookies ?
      net::StaticCookiePolicy::ALLOW_ALL_COOKIES :
      net::StaticCookiePolicy::BLOCK_ALL_THIRD_PARTY_COOKIES;
  net::StaticCookiePolicy policy(policy_type);
  int rv = policy.CanSetCookie(
      request.url(), request.first_party_for_cookies());
  return rv == net::OK;
}

bool 
ExoShellNetworkDelegate::OnCanAccessFile(
    const net::URLRequest& request,
    const base::FilePath& path) const 
{
  return true;
}

bool 
ExoShellNetworkDelegate::OnCanThrottleRequest(
    const net::URLRequest& request) const 
{
  return false;
}

int 
ExoShellNetworkDelegate::OnBeforeSocketStreamConnect(
    net::SocketStream* socket,
    const net::CompletionCallback& callback) 
{
  return net::OK;
}

} // namespace exo_shell
