// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/resource_dispatcher_host_delegate.h"

#include "base/command_line.h"
/* TODO(spolu): introduce ShellLogin Dialog (see content) */
//#include "content/shell/shell_login_dialog.h"

#include "src/common/switches.h"

using namespace content;

namespace thrust_shell {

ThrustShellResourceDispatcherHostDelegate::
  ThrustShellResourceDispatcherHostDelegate() 
{
}

ThrustShellResourceDispatcherHostDelegate::
~ThrustShellResourceDispatcherHostDelegate() 
{
}

ResourceDispatcherHostLoginDelegate*
ThrustShellResourceDispatcherHostDelegate::CreateLoginDelegate(
    net::AuthChallengeInfo* auth_info, 
    net::URLRequest* request) 
{
  /* TODO(spolu): introduce ShellLogin Dialog (see content) */
  return NULL;
}

} // namespace thrust_shell
