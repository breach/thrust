// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/session/thrust_session_proxy_config_service.h"

#include "net/proxy/proxy_service.h"
#include "net/proxy/proxy_config_service_fixed.h"
#include "content/public/browser/browser_thread.h"

#include "src/browser/session/thrust_session.h"

using namespace content;

namespace thrust_shell {

ThrustSessionProxyConfigService::ThrustSessionProxyConfigService(
    ThrustSession* parent)
: parent_(parent)
{
  LOG(INFO) << "ThrustSesionProxyConfigService Constructor: " << this;

  system_service_.reset(
      net::ProxyService::CreateSystemProxyConfigService(
        BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::IO)
          ->message_loop_proxy().get(),
        BrowserThread::UnsafeGetMessageLoopForThread(BrowserThread::FILE)));
  fixed_service_.reset();
}

ThrustSessionProxyConfigService::~ThrustSessionProxyConfigService()
{
  /* We nullify the proxy service in parent to prevent erroneous access */
  /* after it gets reclaimed.                                           */
  parent_->proxy_config_service_ = NULL;

  LOG(INFO) << "ThrustSesionProxyConfigService Destructor: " << this;
}

/******************************************************************************/
/* PUBLIC API */
/******************************************************************************/
void 
ThrustSessionProxyConfigService::SetProxyRules(
    std::string& proxy_string)
{
  net::ProxyConfig proxy_config;
  proxy_config.proxy_rules().ParseFromString(proxy_string);  
  fixed_service_.reset(new net::ProxyConfigServiceFixed(proxy_config));

  LOG(INFO) << "ThrustSesionProxyConfigService SetProxyRules: " << proxy_string;

  if(observers_.might_have_observers()) {
    ObserverList<Observer>::Iterator it(observers_);
    Observer* obs;
    while((obs = it.GetNext()) != NULL) {
      fixed_service_->AddObserver(obs);
    }
  }
  FOR_EACH_OBSERVER(Observer, observers_, 
                    OnProxyConfigChanged(proxy_config, CONFIG_VALID));
}

void 
ThrustSessionProxyConfigService::ClearProxyRules()
{
  fixed_service_.reset();

  net::ProxyConfig proxy_config;
  net::ProxyConfigService::ConfigAvailability avail = 
    this->GetLatestProxyConfig(&proxy_config);

  FOR_EACH_OBSERVER(Observer, observers_, 
                    OnProxyConfigChanged(proxy_config, avail));
}

/******************************************************************************/
/* PROXY CONFIG SERVICE IMPLEMENTATION */
/******************************************************************************/
void 
ThrustSessionProxyConfigService::AddObserver(
  Observer* observer)
{
  observers_.AddObserver(observer);
  if(fixed_service_.get()) {
    fixed_service_->AddObserver(observer);
  }
  system_service_->AddObserver(observer);
}

void 
ThrustSessionProxyConfigService::RemoveObserver(
  Observer* observer)
{
  observers_.RemoveObserver(observer);
  if(fixed_service_.get()) {
    fixed_service_->RemoveObserver(observer);
  }
  system_service_->RemoveObserver(observer);
}

net::ProxyConfigService::ConfigAvailability 
ThrustSessionProxyConfigService::GetLatestProxyConfig(
  net::ProxyConfig* config)
{
  if(fixed_service_.get()) {
    return fixed_service_->GetLatestProxyConfig(config);
  }
  return system_service_->GetLatestProxyConfig(config);
}

void 
ThrustSessionProxyConfigService::OnLazyPoll()
{
  if(fixed_service_.get()) {
    fixed_service_->OnLazyPoll();
  }
  system_service_->OnLazyPoll();
}

} // namespace thrust_shell
