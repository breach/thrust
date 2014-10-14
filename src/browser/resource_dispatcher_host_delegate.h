// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define THRUST_SHELL_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "base/compiler_specific.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace thrust_shell {

class ThrustShellResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  ThrustShellResourceDispatcherHostDelegate();
  virtual ~ThrustShellResourceDispatcherHostDelegate();

  // ResourceDispatcherHostDelegate implementation.
  virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info, 
      net::URLRequest* request) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(ThrustShellResourceDispatcherHostDelegate);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
