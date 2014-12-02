// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_SESSION_THRUST_SESSION_PROXY_CONFIG_SERVICE_H_
#define THRUST_SHELL_BROWSER_SESSION_THRUST_SESSION_PROXY_CONFIG_SERVICE_H_

#include "base/memory/scoped_ptr.h"
#include "base/observer_list.h"
#include "net/proxy/proxy_config_service.h"

namespace thrust_shell {

class ThrustSession;

// ### ThrustSessionProxyConfigService
//
// The ThrustSessionProxyConfigService is the glue between the system proxy 
// service and custom proxy rules set by the user for a given thrust_session. 
// This proxy service is passed as argument to the 
// ThrustShellURLRequestContextGetter to manage all proxy information.
//
// The ProxyConfigService expose two simple methods SetProxyString and 
// ClearProxyString to set or clear (return to system settings) the proxy rules 
// for all requests made for this session
class ThrustSessionProxyConfigService : public net::ProxyConfigService {
public:
  // ### THrustSessionProxyConfigService
  ThrustSessionProxyConfigService(ThrustSession* parent);
  ~ThrustSessionProxyConfigService();

  /****************************************************************************/
  /* PUBLIC API */
  /****************************************************************************/
  void SetProxyRules(std::string& rules);
  void ClearProxyRules();

  /****************************************************************************/
  /* PROXY CONFIG SERVICE IMPLEMENTATION */
  /****************************************************************************/
  virtual void AddObserver(Observer* observer) OVERRIDE;
  virtual void RemoveObserver(Observer* observer) OVERRIDE;

  virtual ConfigAvailability GetLatestProxyConfig(
      net::ProxyConfig* config) OVERRIDE;
  virtual void OnLazyPoll() OVERRIDE;

private:
  ThrustSession*                        parent_;

  ObserverList<Observer>                observers_;
  scoped_ptr<net::ProxyConfigService>   system_service_;
  scoped_ptr<net::ProxyConfigService>   fixed_service_;

  friend class ThrustSession;

  DISALLOW_COPY_AND_ASSIGN(ThrustSessionProxyConfigService);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_SESSION_THRUST_SESSION_PROXY_CONFIG_SERVICE_H_
