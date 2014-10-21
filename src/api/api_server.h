// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_API_API_SERVER_H_
#define THRUST_SHELL_API_API_SERVER_H_

#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

#include "base/files/file_path.h"

#include "src/api/api_binding.h"

namespace base {
class Thread;
class Value;
class DictionaryValue;
}

namespace net {
class StreamListenSocketFactory;
class UnixDomainSocket;
}

namespace thrust_shell {

class API;

class APIServer : public base::RefCountedThreadSafe<APIServer> {
public:

  /****************************************************************************/
  /* APISERVER::CLIENT */
  /****************************************************************************/
  class Client : public base::RefCountedThreadSafe<APIServer::Client> {
  public:
    Client(APIServer* server, API* api);
    ~Client();

    void ProcessChunk(std::string chunk);

    void ReplyToAction(const unsigned int id, 
                       const std::string& error, 
                       scoped_ptr<base::DictionaryValue> result);
  
    /**************************************************************************/
    /* APISERVER::CLIENT::REMOTE */
    /**************************************************************************/
    class Remote : public APIBindingRemote,
                   public base::RefCountedThreadSafe<APIServer::Client::Remote> {
    public:
      Remote(APIServer::Client* client,
             unsigned int target);

      virtual void InvokeMethod(const std::string method,
                                scoped_ptr<base::DictionaryValue> args,
                                const API::MethodCallback& callback) OVERRIDE;
      virtual void EmitEvent(const std::string type,
                             scoped_ptr<base::DictionaryValue> event) OVERRIDE;

    private:
      APIServer::Client*                      client_;

      unsigned int                            target_;

      DISALLOW_COPY_AND_ASSIGN(Remote);
    };

  private:
    void PerformAction(scoped_ptr<base::DictionaryValue> action);

    void SendReply(const unsigned id,
                   const std::string& error,
                   scoped_ptr<base::DictionaryValue> result);
    void SendInvoke(const API::MethodCallback& callback,
                    unsigned int target,
                    const std::string method,
                    scoped_ptr<base::DictionaryValue> args);
    void SendEvent(unsigned int target,
                   const std::string type,
                   scoped_ptr<base::DictionaryValue> event);
                     

    APIServer*                                         server_;
    API*                                               api_;
    unsigned int                                       action_id_;

    std::string                                        acc_;

    std::map<unsigned int, scoped_refptr<Remote> >     remotes_;
    std::map<int, API::MethodCallback >                invokes_;

    DISALLOW_COPY_AND_ASSIGN(Client);
  };

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  APIServer(API* api);

  // ### Start
  // 
  // Starts the APIServer server on its own thread listening
  void Start();

  // ### Stop
  //
  // Stops the APIServer and shuts down the server
  void Stop();

private:
  /****************************************************************************/
  /* PRIVATE INTERFACE */
  /****************************************************************************/
  void StartHandlerThread();
  void ResetHandlerThread();
  void StopHandlerThread();
  void ThreadRun();
  void ThreadTearDown();

  void DestroyClient();

  /* The thread used by the API handler to run server socket. */
  API*                                                       api_;
  scoped_ptr<base::Thread>                                   thread_;

  scoped_refptr<Client>                                      client_;

  DISALLOW_COPY_AND_ASSIGN(APIServer);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_API_API_SERVER_H_
