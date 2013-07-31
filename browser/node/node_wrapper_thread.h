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

    // Main Node initialization code. It sets up the v8 context and initialize
    // all node related stuff
    void Initialize();

    // This is the code which runs the node run loop. It blocks until one
    // uv event has been processed. As soon as uv returns, a PostTask is
    // called on itself
    void RunUvLoop();

    v8::Local<v8::Object> process_;
};

}

#endif // BREACH_BROWSER_NODE_NODE_WRAPPER_THREAD_H_
