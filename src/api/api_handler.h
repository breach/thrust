// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_API_API_HANDLER_H_
#define EXO_BROWSER_API_API_HANDLER_H_

namespace net {
class StreamListenSocketFactory;
}

namespace exo_browser {

class ApiHandler {

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  // ### Start
  // 
  // Starts the ApiHandler server on its own thread listening on the socket
  // created by the `socket_factory` passed as argument
  void Start(const net::StreamListenSocketFactory* socket_factory);

  // ### InstallBindings
  //
  // Installs a new binding for the API

  /* The thread used by the API handler to run server socket. */
  scoped_ptr<base::Thread> thread_;

  DISALLOW_COPY_AND_ASSIGN(ApiHandler);
};

}

#endif // EXO_BROWSER_API_API_HANDLER_H_
