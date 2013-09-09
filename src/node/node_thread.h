// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_NODE_NODE_THREAD_H_
#define EXO_BROWSER_NODE_NODE_THREAD_H_

#include "base/threading/thread.h"
#include "v8/include/v8.h"
#include "third_party/node/src/node.h"

namespace exo_browser {

class ApiBindings;

class NodeThread : public base::Thread {
public:
  static NodeThread* Get();

  // ### PostTask
  // Wrapper to post a task on the thread event loop while making sure of 
  // waking up the uv run loop by sending a dummy async handle. Posting using
  // directy the thread event loop may not get executed (or with a substantial
  // delay as the thread is most of the time waiting for UV events
  void PostTask(const tracked_objects::Location& from_here,
                const base::Closure& task);

protected:
  virtual void Init() OVERRIDE;
  virtual void Run(base::MessageLoop* message_loop) OVERRIDE;
  virtual void CleanUp() OVERRIDE;


 private:
  NodeThread();
  virtual ~NodeThread();

  // This is the code which runs the node run loop. It blocks until one
  // uv event has been processed. As soon as uv returns, a PostTask is
  // called on itself
  void RunUvLoop();

  // Be careful: these objects only live while the message_loop is running
  v8::Handle<v8::Object> process_;
  v8::Handle<v8::Context> context_;

  // Api Bindings kept in memory
  scoped_ptr<ApiBindings> api_bindings_;
  uv_async_t              uv_dummy;

  DISALLOW_COPY_AND_ASSIGN(NodeThread);
};

} // namespace exo_browser

#endif // EXO_BROWSER_NODE_NODE_THREAD_H_
