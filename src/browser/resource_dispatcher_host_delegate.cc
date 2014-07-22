// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/resource_dispatcher_host_delegate.h"

#include "base/command_line.h"
/* TODO(spolu): introduce ShellLogin Dialog (see content) */
//#include "content/shell/shell_login_dialog.h"

#include "src/common/switches.h"

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

ResourceDispatcherHostLoginDelegate*
ExoBrowserResourceDispatcherHostDelegate::CreateLoginDelegate(
    net::AuthChallengeInfo* auth_info, 
    net::URLRequest* request) 
{
  /* TODO(spolu): introduce ShellLogin Dialog (see content) */
  return NULL;
}

} // namespace exo_browser
