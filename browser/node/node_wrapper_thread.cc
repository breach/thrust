// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "breach/browser/node/node_wrapper_thread.h"
#include "content/public/browser/browser_thread.h"
#include "third_party/node/src/node.h"
#include "third_party/node/src/node_internals.h"

using v8::Isolate;
using v8::HandleScope;
using v8::Local;
using v8::Locker;
using v8::Context;
using v8::Object;
using v8::V8;
using v8::Value;


namespace breach {

NodeWrapperThread::NodeWrapperThread()
: Thread("node_wrapper_thread")
{
  Initialize();
}

NodeWrapperThread::~NodeWrapperThread()
{
  /* All Thread subclasses must call Stop() in the destructor */
  Stop();
}

void 
NodeWrapperThread::Init() 
{
  message_loop()->PostTask(FROM_HERE,
                           base::Bind(&NodeWrapperThread::RunUvLoop,
                                      base::Unretained(this)));
}

void 
NodeWrapperThread::Run(
    base::MessageLoop* message_loop) 
{
  int argc = 1;
  char* argv[] = { const_cast<char*>("node"), NULL, NULL }; 

  node::InitSetup(argc, argv);
  Isolate* node_isolate = Isolate::GetCurrent();

  V8::Initialize();
  {
    Locker locker(node_isolate);
    HandleScope handle_scope(node_isolate);

    // Create the one and only Context.
    context_ = Context::New(node_isolate);
    Context::Scope context_scope(context_);

    node::SetupBindingCache();
    process_ = node::SetupProcessObject(argc, argv);

    // Create all the objects, load modules, do everything.
    // so your next reading stop should be node::Load()!
    node::Load(process_);

    Thread::Run(message_loop);

    node::EmitExit(process_);
    node::RunAtExit();
  }
}

void
NodeWrapperThread::CleanUp()
{
  /* Clean up. Not strictly necessary. */
  //V8::Dispose();
}


void
NodeWrapperThread::Initialize() 
{
}

void
NodeWrapperThread::RunUvLoop()
{
  int ret = uv_run(uv_default_loop(), UV_RUN_ONCE);
  if(ret > 0) {
    /* Recursively call */
    message_loop()->PostTask(FROM_HERE,
                             base::Bind(&NodeWrapperThread::RunUvLoop,
                                        base::Unretained(this)));
  }
  else {
    LOG(INFO) << "Node Exit";
    //node::EmitExit(process_);
  }
}

} // namespace breach
