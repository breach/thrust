// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "breach/browser/node/node_thread.h"

#include "breach/browser/node/api/api_bindings.h"
#include "content/public/browser/browser_thread.h"
#include "third_party/node/src/node.h"
#include "third_party/node/src/node_internals.h"
#include "base/command_line.h"
#include "base/time/time.h"

using v8::Isolate;
using v8::HandleScope;
using v8::Local;
using v8::Locker;
using v8::Context;
using v8::Object;
using v8::V8;
using v8::Value;
using v8::Script;
using v8::String;
using v8::RegisterExtension;

using namespace content;

namespace breach {

static NodeThread* s_thread = NULL;

NodeThread*
NodeThread::Get()
{
  if(s_thread == NULL) {
    s_thread = new NodeThread();
  }
  return s_thread;
}

NodeThread::NodeThread()
: Thread("node_wrapper_thread")
{
}

NodeThread::~NodeThread()
{
  /* All Thread subclasses must call Stop() in the destructor */
  Stop();
}

void 
NodeThread::Init() 
{
  message_loop()->PostTask(FROM_HERE,
                           base::Bind(&NodeThread::RunUvLoop,
                                      base::Unretained(this)));
}

void 
NodeThread::Run(
    base::MessageLoop* message_loop) 
{
  /* TODO(spolu): fork execution depending on kBreachRawInit */
  /* If not set, launch the default version of the Browser.  */
  /* If set, pass argc/argv to Node                          */

  /* Extract argc, argv to pass it directly to Node */
  const CommandLine* command_line = CommandLine::ForCurrentProcess();
  int argc = command_line->argv().size();
  char **argv = (char**)malloc(argc * sizeof(char*));
  for(int i = 0; i < argc; i ++) {
    unsigned len = strlen(command_line->argv()[i].c_str()) + 1;
    argv[i] = (char*) malloc(len * sizeof(char));
    memcpy(argv[i], command_line->argv()[i].c_str(), len);
  }

  node::InitSetup(argc, argv);
  Isolate* node_isolate = Isolate::GetCurrent();

  V8::Initialize();
  {
    Locker locker(node_isolate);
    HandleScope handle_scope(node_isolate);

    api_bindings_.reset(new ApiBindings());
    RegisterExtension(api_bindings_.get());
    const char* names[] = { "api_bindings.js" };
    v8::ExtensionConfiguration extensions(1, names);  

    /* Create the one and only Context. */
    context_ = Context::New(node_isolate, &extensions);
    Context::Scope context_scope(context_);

    node::SetupBindingCache();
    process_ = node::SetupProcessObject(argc, argv);

    /* Create all the objects, load modules, do everything. */
    /* so your next reading stop should be node::Load()!    */
    node::Load(process_);

    Thread::Run(message_loop);

    node::EmitExit(process_);
    node::RunAtExit();
  }

  /* Cleanup */
  for(int i = 0; i < argc; i++) { free(argv[i]); }
  free(argv);
}

void
NodeThread::CleanUp()
{
  /* Clean up. Not strictly necessary. */
  V8::Dispose();
}


void
NodeThread::RunUvLoop()
{
  /* int ret = */ uv_run(uv_default_loop(), UV_RUN_NOWAIT);

  /* TODO(spolu): FixMe [/!\ Bad Code Ahead]                                 */
  /* If we call with UV_RUN_ONCE then it will hang the thread while there is */
  /* no `uv` related events, blocking all Chromium/Breach message delivery.  */
  /* We therefore post a delayed task to avoid hogging the CPU but not too   */
  /* far away to avoid any delay in the execution of nodeJS code. This is no */
  /* perfect, but it works!                                                  */
  /* Eventual best solution will be to implement a message_loop based on uv  */
  /* as it has already been done for node-webkit                             */
  /* Another acceptable solution would be to run the uv run_loop direclty    */
  /* from here and expose a mechanism for existing content threads to post a */
  /* task on that thread.                                                    */
  message_loop()->PostDelayedTask(FROM_HERE,
                                  base::Bind(&NodeThread::RunUvLoop,
                                             base::Unretained(this)),
                                  base::TimeDelta::FromMicroseconds(100));
}

} // namespace breach
