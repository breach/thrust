// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
#define EXO_BROWSER_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_

#include "base/compiler_specific.h"
#include "content/public/browser/resource_dispatcher_host_delegate.h"

namespace exo_browser {

class ExoBrowserResourceDispatcherHostDelegate
    : public content::ResourceDispatcherHostDelegate {
 public:
  ExoBrowserResourceDispatcherHostDelegate();
  virtual ~ExoBrowserResourceDispatcherHostDelegate();

  // ResourceDispatcherHostDelegate implementation.
  virtual bool AcceptAuthRequest(net::URLRequest* request,
                                 net::AuthChallengeInfo* auth_info) OVERRIDE;
  virtual content::ResourceDispatcherHostLoginDelegate* CreateLoginDelegate(
      net::AuthChallengeInfo* auth_info, 
      net::URLRequest* request) OVERRIDE;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserResourceDispatcherHostDelegate);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_RESOURCE_DISPATCHER_HOST_DELEGATE_H_
