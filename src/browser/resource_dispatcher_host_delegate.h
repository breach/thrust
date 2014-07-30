// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define EXO_SHELL_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "base/compiler_specific.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace exo_shell {

class ExoShellResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  ExoShellResourceDispatcherHostDelegate();
  virtual ~ExoShellResourceDispatcherHostDelegate();

  // ResourceDispatcherHostDelegate implementation.
  virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info, 
      net::URLRequest* request) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(ExoShellResourceDispatcherHostDelegate);
};

} // namespace exo_shell

#endif // EXO_SHELL_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
