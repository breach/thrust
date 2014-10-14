// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/api/thrust_session_binding.h"

#include "src/browser/session/thrust_session.h"

namespace thrust_shell {

ThrustSessionBindingFactory::ThrustSessionBindingFactory()
{
}

ThrustSessionBindingFactory::~ThrustSessionBindingFactory()
{
}

APIBinding* ThrustSessionBindingFactory::Create(
    const unsigned int id,
    scoped_ptr<base::DictionaryValue> args)
{
  return new ThrustSessionBinding(id, args.Pass());
}


ThrustSessionBinding::ThrustSessionBinding(
    const unsigned int id, 
    scoped_ptr<base::DictionaryValue> args)
  : APIBinding("session", id)
{
  bool off_the_record = true;
  args->GetBoolean("off_the_record", &off_the_record);

  std::string path = "dummy_session";
  args->GetString("path", &path);

  bool dummy_cookie_store = true;
  args->GetBoolean("dummy_cookie_store", &dummy_cookie_store);

  session_.reset(new ThrustSession(off_the_record, 
                                path, 
                                dummy_cookie_store));
  session_->Initialize();
}

ThrustSessionBinding::~ThrustSessionBinding()
{
  LOG(INFO) << "ThrustSessionBinding Destructor";
  session_.reset();
}


void
ThrustSessionBinding::CallLocalMethod(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const API::MethodCallback& callback)
{
  std::string err = std::string("");
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "CALL " << method;
  if(method.compare("devtools_url") == 0) {
    res->SetString("url", session_->GetDevToolsURL().spec());
  }
  else if(method.compare("off_the_record") == 0) {
    res->SetBoolean("off_the_record", session_->IsOffTheRecord());
  }
  else {
    err = "exo_session_binding:method_not_found";
  }

  callback.Run(err, scoped_ptr<base::Value>(res).Pass());
}

ThrustSession*
ThrustSessionBinding::GetSession() {
  return session_.get();
}


} // namespace thrust_shell
