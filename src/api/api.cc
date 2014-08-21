// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/api/api_server.h"

#include "base/callback.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/values.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"

#include "src/api/api_binding.h"

using namespace content;

namespace exo_shell {

// static
API* API::self_ = NULL;

API::API() 
  : next_binding_id_(0)
{
  DCHECK(!self_) << "Cannot have two API Instances";
  self_ = this;
}

// static
API* 
API::Get() 
{
  DCHECK(self_);
  return self_;
}

API::~API() 
{
}

void
API::InstallBinding(
    const std::string& type,
    APIBindingFactory* factory)
{
  LOG(INFO) << "[API] INSTALL: " << type;
  factories_[type] = factory;
}

int
API::Create(
    const std::string type,
    scoped_ptr<base::DictionaryValue> args)
{
  int target = 0;
  if(type.length() > 0 && factories_[type]) {
    /* We call into the binding factory to create the binding. This will */
    /* trigger the creation of a local object.                           */
    target = ++next_binding_id_;
    LOG(INFO) << "[API] CREATE: " << type << " " << target;
    bindings_[target] = factories_[type]->Create(target, args.Pass());
  }
  return 0;
}

void
API::Delete(
    unsigned int target)
{
  if(target > 0 && bindings_[target]) {
    LOG(INFO) << "[API] DELETE: " << target;
    /* We call the destructor of the binding which is in charge of */
    /* cleaning up all local objects associated with it.           */
    delete bindings_[target];
    bindings_.erase(target);

    /* Finally we delete the remote object */
    if(remotes_[target]) {
      delete remotes_[target];
      remotes_.erase(target);
    }
  }
}

void 
API::CallMethod(
    unsigned int target,
    std::string method,
    scoped_ptr<base::DictionaryValue> args,
    const API::MethodCallback& callback)
{
  if(target > 0 && bindings_[target]) {
    LOG(INFO) << "[API] CALL: " << target << " " << method;
    /* We route the request to the right binding. */
    bindings_[target]->LocalCall(method, args.Pass(), callback);
  }
}

void 
API::SetRemote(
    unsigned int target,
    APIBindingRemote* remote)
{
  if(target > 0 && bindings_[target]) {
    remotes_[target] = remote;
  }
}

