// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/net/url_request_context_getter.h"

#include "base/command_line.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/worker_pool.h"
#include "net/base/cache_type.h"
#include "net/cert/cert_verifier.h"
#include "net/cookies/cookie_monster.h"
#include "net/dns/host_resolver.h"
#include "net/dns/mapped_host_resolver.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/http/http_network_session.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/transport_security_state.h"
#include "net/proxy/dhcp_proxy_script_fetcher_factory.h"
#include "net/proxy/proxy_service.h"
#include "net/ssl/default_server_bound_cert_store.h"
#include "net/ssl/server_bound_cert_service.h"
#include "net/ssl/ssl_config_service_defaults.h"
#include "net/url_request/data_protocol_handler.h"
#include "net/url_request/file_protocol_handler.h"
#include "net/url_request/protocol_intercept_job_factory.h"
#include "net/url_request/static_http_user_agent_settings.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_storage.h"
#include "net/url_request/url_request_job_factory_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "content/public/common/url_constants.h"
#include "exo_browser/src/common/switches.h"
#include "exo_browser/src/net/network_delegate.h"
#include "exo_browser/src/browser/session/exo_session.h"

using namespace content;

namespace exo_browser {

namespace {

void InstallProtocolHandlers(net::URLRequestJobFactoryImpl* job_factory,
                             ProtocolHandlerMap* protocol_handlers) {
  for (ProtocolHandlerMap::iterator it =
           protocol_handlers->begin();
       it != protocol_handlers->end();
       ++it) {
    bool set_protocol = job_factory->SetProtocolHandler(
        it->first, it->second.release());
    DCHECK(set_protocol);
  }
  protocol_handlers->clear();
}

}  // namespace

ExoBrowserURLRequestContextGetter::ExoBrowserURLRequestContextGetter(
    ExoSession* parent,
    bool ignore_certificate_errors,
    const base::FilePath& base_path,
    base::MessageLoop* io_loop,
    base::MessageLoop* file_loop,
    ProtocolHandlerMap* protocol_handlers,
    ProtocolHandlerScopedVector protocol_interceptors,
    net::NetLog* net_log)
    : parent_(parent),
      ignore_certificate_errors_(ignore_certificate_errors),
      base_path_(base_path),
      io_loop_(io_loop),
      file_loop_(file_loop),
      net_log_(net_log),
      protocol_interceptors_(protocol_interceptors.Pass())
{
  // Must first be created on the UI thread.
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  std::swap(protocol_handlers_, *protocol_handlers);
  
  // We must create the proxy config service on the UI loop on Linux because it
  // must synchronously run on the glib message loop. This will be passed to
  // the URLRequestContextStorage on the IO thread in GetURLRequestContext().
  proxy_config_service_.reset(
      net::ProxyService::CreateSystemProxyConfigService(
        io_loop_->message_loop_proxy().get(), file_loop_));
}

ExoBrowserURLRequestContextGetter::~ExoBrowserURLRequestContextGetter() 
{
}

net::URLRequestContext* 
ExoBrowserURLRequestContextGetter::GetURLRequestContext() 
{
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));

  if (!url_request_context_) {
    url_request_context_.reset(new net::URLRequestContext());
    url_request_context_->set_net_log(net_log_);
    network_delegate_.reset(new ExoBrowserNetworkDelegate);
    url_request_context_->set_network_delegate(network_delegate_.get());
    storage_.reset(
        new net::URLRequestContextStorage(url_request_context_.get()));

    scoped_refptr<net::CookieStore> cookie_store = NULL;
    if(parent_) {
      cookie_store = new net::CookieMonster(parent_->GetCookieStore(), NULL);
    }
    else {
      cookie_store = new net::CookieMonster(NULL, NULL);
    }
    cookie_store->GetCookieMonster()->SetPersistSessionCookies(true);
    storage_->set_cookie_store(cookie_store);

    const char* schemes[] = {"http", "https", "file", "app"};
    cookie_store->GetCookieMonster()->SetCookieableSchemes(schemes, 4);
          
    storage_->set_server_bound_cert_service(new net::ServerBoundCertService(
        new net::DefaultServerBoundCertStore(NULL),
        base::WorkerPool::GetTaskRunner(true)));
    storage_->set_http_user_agent_settings(
        new net::StaticHttpUserAgentSettings("en-us,en", std::string()));

    scoped_ptr<net::HostResolver> host_resolver(
        net::HostResolver::CreateDefaultResolver(
            url_request_context_->net_log()));

    storage_->set_cert_verifier(net::CertVerifier::CreateDefault());
    storage_->set_transport_security_state(new net::TransportSecurityState);

    /* TODO(spolu): use v8 if possible, look at chrome code. */
    storage_->set_proxy_service(
        net::ProxyService::CreateUsingSystemProxyResolver(
          proxy_config_service_.release(),
          0,
          url_request_context_->net_log()));

    storage_->set_ssl_config_service(new net::SSLConfigServiceDefaults);
    storage_->set_http_auth_handler_factory(
        net::HttpAuthHandlerFactory::CreateDefault(host_resolver.get()));
    storage_->set_http_server_properties(
        scoped_ptr<net::HttpServerProperties>(
            new net::HttpServerPropertiesImpl()));


    net::HttpCache::BackendFactory* main_backend = NULL;
    if(parent_->IsOffTheRecord()) {
      main_backend = net::HttpCache::DefaultBackend::InMemory(0);
    }
    else {
      base::FilePath cache_path = base_path_.Append(FILE_PATH_LITERAL("Cache"));
      main_backend =
        new net::HttpCache::DefaultBackend(
            net::DISK_CACHE,
            net::CACHE_BACKEND_DEFAULT,
            cache_path,
            0,
            BrowserThread::GetMessageLoopProxyForThread(BrowserThread::CACHE)
                .get());
    }

    net::HttpNetworkSession::Params network_session_params;
    network_session_params.cert_verifier =
        url_request_context_->cert_verifier();
    network_session_params.transport_security_state =
        url_request_context_->transport_security_state();
    network_session_params.server_bound_cert_service =
        url_request_context_->server_bound_cert_service();
    network_session_params.proxy_service =
        url_request_context_->proxy_service();
    network_session_params.ssl_config_service =
        url_request_context_->ssl_config_service();
    network_session_params.http_auth_handler_factory =
        url_request_context_->http_auth_handler_factory();
    network_session_params.network_delegate =
        network_delegate_.get();
    network_session_params.http_server_properties =
        url_request_context_->http_server_properties();
    network_session_params.net_log =
        url_request_context_->net_log();
    network_session_params.ignore_certificate_errors =
        ignore_certificate_errors_;

    // Give |storage_| ownership at the end in case it's |mapped_host_resolver|.
    storage_->set_host_resolver(host_resolver.Pass());
    network_session_params.host_resolver =
        url_request_context_->host_resolver();

    net::HttpCache* main_cache = new net::HttpCache(
        network_session_params, main_backend);
    storage_->set_http_transaction_factory(main_cache);

    scoped_ptr<net::URLRequestJobFactoryImpl> job_factory(
        new net::URLRequestJobFactoryImpl());
    // Keep ProtocolHandlers added in sync with
    // ExoBrowserContentBrowserClient::IsHandledURL().
    InstallProtocolHandlers(job_factory.get(), &protocol_handlers_);
    bool set_protocol = job_factory->SetProtocolHandler(
        kDataScheme, new net::DataProtocolHandler);
    DCHECK(set_protocol);
    set_protocol = job_factory->SetProtocolHandler(
        kFileScheme,
        new net::FileProtocolHandler(
            content::BrowserThread::GetBlockingPool()->
                GetTaskRunnerWithShutdownBehavior(
                    base::SequencedWorkerPool::SKIP_ON_SHUTDOWN)));
    DCHECK(set_protocol);

    // Set up interceptors in the reverse order.
    scoped_ptr<net::URLRequestJobFactory> top_job_factory =
        job_factory.PassAs<net::URLRequestJobFactory>();
    for (ProtocolHandlerScopedVector::reverse_iterator i =
             protocol_interceptors_.rbegin();
         i != protocol_interceptors_.rend();
         ++i) {
      top_job_factory.reset(new net::ProtocolInterceptJobFactory(
          top_job_factory.Pass(), make_scoped_ptr(*i)));
    }
    protocol_interceptors_.weak_clear();

    storage_->set_job_factory(top_job_factory.release());
  }

  return url_request_context_.get();
}

scoped_refptr<base::SingleThreadTaskRunner>
ExoBrowserURLRequestContextGetter::GetNetworkTaskRunner() const 
{
  return BrowserThread::GetMessageLoopProxyForThread(BrowserThread::IO);
}

net::HostResolver* 
ExoBrowserURLRequestContextGetter::host_resolver() 
{
  return url_request_context_->host_resolver();
}

} // namespace exo_browser
