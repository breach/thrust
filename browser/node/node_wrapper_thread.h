// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_NODE_NODE_WRAPPER_THREAD_H_
#define BREACH_BROWSER_NODE_NODE_WRAPPER_THREAD_H_

#include "base/threading/thread.h"
#include "v8/include/v8.h"

namespace breach {

class NodeWrapperThread : public base::Thread {
  public:

    NodeWrapperThread();
    virtual ~NodeWrapperThread();

  protected:
    virtual void Init() OVERRIDE;
    virtual void Run(base::MessageLoop* message_loop) OVERRIDE;
    virtual void CleanUp() OVERRIDE;


  private:

    // Installs all the Node symbols provided by Breach
    void InstallNodeSymbols();

    // This is the code which runs the node run loop. It blocks until one
    // uv event has been processed. As soon as uv returns, a PostTask is
    // called on itself
    void RunUvLoop();

    // Be careful: these objects only live while the message_loop is running
    v8::Handle<v8::Object> process_;
    v8::Handle<v8::Context> context_;
};

}

#endif // BREACH_BROWSER_NODE_NODE_WRAPPER_THREAD_H_
