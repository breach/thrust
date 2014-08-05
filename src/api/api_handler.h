// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_API_API_HANDLER_H_
#define EXO_SHELL_API_API_HANDLER_H_

#include <map>

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

#include "base/files/file_path.h"
#include "net/socket/stream_listen_socket.h"
#include "net/socket/socket_descriptor.h"
#include "net/socket/unix_domain_socket_posix.h"

namespace base {
class Thread;
class Value;
class DictionaryValue;
}

namespace net {
class StreamListenSocketFactory;
class UnixDomainSocket;
}

namespace exo_shell {

class ApiBinding;
class ApiBindingFactory;

class ApiHandler : public net::StreamListenSocket::Delegate,
                   public base::RefCountedThreadSafe<ApiHandler> {
public:
  typedef base::Callback<void(const std::string& error, 
                              scoped_ptr<base::Value> result)> ActionCallback;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  ApiHandler(const base::FilePath& socket_path);

  // ### Start
  // 
  // Starts the ApiHandler server on its own thread listening
  void Start();

  // ### Stop
  //
  // Stops the ApiHandler and shuts down the server
  void Stop();

  // ### InstallBinding
  //
  // Installs a new binding for the API
  void InstallBinding(const std::string& type,
                      ApiBindingFactory* factory);

  /****************************************************************************/
  /* STREAMLISTENSOCKET::DELEGATE INTERFACE */
  /****************************************************************************/
  virtual void DidAccept(net::StreamListenSocket* server,                          
                         scoped_ptr<net::StreamListenSocket> connection) OVERRIDE;
  virtual void DidRead(net::StreamListenSocket* connection,
                       const char* data,
                       int len) OVERRIDE;
  virtual void DidClose(net::StreamListenSocket* sock) OVERRIDE;

private:
  /****************************************************************************/
  /* PRIVATE INTERFACE */
  /****************************************************************************/
  void ReplyToAction(const unsigned int id, 
                     const std::string& error, 
                     scoped_ptr<base::Value> result);

  void PerformAction(std::string action,
                     unsigned int id,
                     scoped_ptr<base::DictionaryValue> args,
                     unsigned int target,
                     std::string type,
                     std::string method);

  void SendReply(const unsigned id,
                 const std::string& error,
                 scoped_ptr<base::Value> result);
  void ProcessData();

  bool UserCanConnectCallback(uid_t user_id, gid_t group_id);
  void DeleteSocketFile();

  void StartHandlerThread();
  void ResetHandlerThread();
  void StopHandlerThread();
  void ThreadInit();
  void ThreadTearDown();

  /* The thread used by the API handler to run server socket. */
  scoped_ptr<base::Thread>                  thread_;

  const base::FilePath                      socket_path_;
  scoped_ptr<net::UnixDomainSocket>         socket_;
  scoped_ptr<net::StreamListenSocket>       conn_;

  std::string                               acc_;

  unsigned int                              next_binding_id_;
  std::map<std::string, ApiBindingFactory*> factories_;
  std::map<unsigned int, ApiBinding*>       bindings_;

  DISALLOW_COPY_AND_ASSIGN(ApiHandler);
};

}

#endif // EXO_SHELL_API_API_HANDLER_H_
