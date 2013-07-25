// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_BREACH_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define BREACH_BROWSER_BREACH_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "base/compiler_specific.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace breach {

class BreachResourceDispatcherHostDelegate
    : public ResourceDispatcherHostDelegate {
 public:
  BreachResourceDispatcherHostDelegate();
  virtual ~BreachResourceDispatcherHostDelegate();

  // ResourceDispatcherHostDelegate implementation.
  virtual bool AcceptAuthRequest(net::URLRequest* request,
                                 net::AuthChallengeInfo* auth_info) OVERRIDE;
  virtual ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info, 
      net::URLRequest* request) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(BreachResourceDispatcherHostDelegate);
};

}  // namespace breach

#endif  // BREACH_BROWSER_BREACH_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
