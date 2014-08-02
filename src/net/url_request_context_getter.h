// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_SHELL_NET_URL_REQUEST_CONTEXT_GETTER_H_
#define EXO_SHELL_NET_URL_REQUEST_CONTEXT_GETTER_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/browser/content_browser_client.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_job_factory.h"

namespace base {
class MessageLoop;
}

namespace net {
class HostResolver;
class MappedHostResolver;
class NetworkDelegate;
class NetLog;
class ProxyConfigService;
class URLRequestContextStorage;
}

namespace exo_shell {

class ExoSession;

class ExoShellURLRequestContextGetter : public net::URLRequestContextGetter {
 public:
  ExoShellURLRequestContextGetter(
      ExoSession* parent,
      bool ignore_certificate_errors,
      const base::FilePath& base_path,
      base::MessageLoop* io_loop,
      base::MessageLoop* file_loop,
      content::ProtocolHandlerMap* protocol_handlers,
      content::ProtocolHandlerScopedVector protocol_interceptors,
      net::NetLog* net_log);

  // net::URLRequestContextGetter implementation.
  virtual net::URLRequestContext* GetURLRequestContext() OVERRIDE;
  virtual scoped_refptr<base::SingleThreadTaskRunner>
      GetNetworkTaskRunner() const OVERRIDE;

  net::HostResolver* host_resolver();

 protected:
  virtual ~ExoShellURLRequestContextGetter();

 private:
  ExoSession*                               parent_;
  bool                                      ignore_certificate_errors_;
  base::FilePath                            base_path_;
  base::MessageLoop*                        io_loop_;
  base::MessageLoop*                        file_loop_;
  net::NetLog*                              net_log_;

  scoped_ptr<net::ProxyConfigService>       proxy_config_service_;
  scoped_ptr<net::NetworkDelegate>          network_delegate_;
  scoped_ptr<net::URLRequestContextStorage> storage_;
  scoped_ptr<net::URLRequestContext>        url_request_context_;
  content::ProtocolHandlerMap               protocol_handlers_;
  content::ProtocolHandlerScopedVector      protocol_interceptors_;

  friend class ExoSession;

  DISALLOW_COPY_AND_ASSIGN(ExoShellURLRequestContextGetter);
};

} // namespace exo_shell

#endif // EXO_SHELL_NET_URL_REQUEST_CONTEXT_GETTER_H_
