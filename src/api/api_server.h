// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_API_API_SERVER_H_
#define EXO_SHELL_API_API_SERVER_H_

namespace base {
class Thread;
}

namespace net {
class StreamListenSocketFactory;
class UnixDomainSocket;
}

namespace exo_shell {

class API;

class APIServer : public net::StreamListenSocket::Delegate,
                  public base::RefCountedThreadSafe<APIServer> {

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  APIServer(API* api,
            const base::FilePath& socket_path);

  // ### Start
  // 
  // Starts the APIServer server on its own thread listening
  void Start();

  // ### Stop
  //
  // Stops the APIServer and shuts down the server
  void Stop();

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
  API*                                      api_;
  scoped_ptr<base::Thread>                  thread_;

  const base::FilePath                      socket_path_;
  scoped_ptr<net::UnixDomainSocket>         socket_;
  scoped_ptr<net::StreamListenSocket>       conn_;

  std::string                               acc_;

  DISALLOW_COPY_AND_ASSIGN(APIServer);
};

} // namespace exo_shell

#endif // EXO_SHELL_API_API_SERVER_H_
