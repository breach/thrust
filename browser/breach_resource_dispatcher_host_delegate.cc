// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/breach_resource_dispatcher_host_delegate.h"

#include "base/command_line.h"
#include "breach/common/breach_switches.h"
/* TODO(spolu): renaming post file creation */
#include "content/shell/shell_login_dialog.h"

using namespace content;

namespace breach {

BreachResourceDispatcherHostDelegate::BreachResourceDispatcherHostDelegate() 
{
}

BreachResourceDispatcherHostDelegate::~BreachResourceDispatcherHostDelegate() 
{
}

bool 
BreachResourceDispatcherHostDelegate::AcceptAuthRequest(
    net::URLRequest* request,
    net::AuthChallengeInfo* auth_info) 
{
  /* Why not? */
  return true;
}

ResourceDispatcherHostLoginDelegate*
BreachResourceDispatcherHostDelegate::CreateLoginDelegate(
    net::AuthChallengeInfo* auth_info, 
    net::URLRequest* request) 
{
  /* TODO(spolu): introduce ShellLogin Dialog (see content) */
  return NULL;
}

}  // namespace breach
