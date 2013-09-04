// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo/exo_browser/browser/resource_dispatcher_host_delegate.h"

#include "base/command_line.h"
#include "exo/exo_browser/common/switches.h"
/* TODO(spolu): introduce ShellLogin Dialog (see content) */
//#include "content/shell/shell_login_dialog.h"

using namespace content;

namespace exo_browser {

ExoBrowserResourceDispatcherHostDelegate::
  ExoBrowserResourceDispatcherHostDelegate() 
{
}

ExoBrowserResourceDispatcherHostDelegate::
~ExoBrowserResourceDispatcherHostDelegate() 
{
}

bool 
ExoBrowserResourceDispatcherHostDelegate::AcceptAuthRequest(
    net::URLRequest* request,
    net::AuthChallengeInfo* auth_info) 
{
  /* Why not? */
  return true;
}

ResourceDispatcherHostLoginDelegate*
ExoBrowserResourceDispatcherHostDelegate::CreateLoginDelegate(
    net::AuthChallengeInfo* auth_info, 
    net::URLRequest* request) 
{
  /* TODO(spolu): introduce ShellLogin Dialog (see content) */
  return NULL;
}

} // namespace exo_browser
