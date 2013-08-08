// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "breach/browser/node/node_thread.h"

#include "breach/browser/node/api/api_bindings.h"
#include "content/public/browser/browser_thread.h"
#include "third_party/node/src/node.h"
#include "third_party/node/src/node_internals.h"
#include "base/command_line.h"

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
    unsigned len = sizeof command_line->argv()[i].c_str();
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
    InstallNodeSymbols();

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
NodeThread::InstallNodeSymbols()
{
  HandleScope handle_scope(Isolate::GetCurrent());

  Local<Script> script = Script::New(String::New(
        /* Overload require */
        "global._require = global._require || global.require;"
        "global.require = function(name) {"
        "  if (name == 'breach')"
        "    return apiDispatcher.requireBreach();"
        "  return global._require(name);"
        "};"
        "global._breach = require('breach');"

        /* Save node-webkit version */
        "process.versions['breach'] = '" BREACH_VERSION "';"
        ));
    script->Run();
}


void
NodeThread::RunUvLoop()
{
  int ret = uv_run(uv_default_loop(), UV_RUN_ONCE);
  if(ret > 0) {
    /* Recursively call */
    message_loop()->PostTask(FROM_HERE,
                             base::Bind(&NodeThread::RunUvLoop,
                                        base::Unretained(this)));
  }
  else {
    LOG(INFO) << "Node Exit";
    node::EmitExit(process_);
  }
}

} // namespace breach
