// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/api/exo_session_binding.h"

#include "src/browser/session/exo_session.h"

namespace exo_shell {

ExoSessionBindingFactory::ExoSessionBindingFactory()
{
}

ExoSessionBindingFactory::~ExoSessionBindingFactory()
{
}

ApiBinding* ExoSessionBindingFactory::Create(
    const unsigned int id,
    scoped_ptr<base::DictionaryValue> args)
{
  return new ExoSessionBinding(id, args.Pass());
}


ExoSessionBinding::ExoSessionBinding(
    const unsigned int id, 
    scoped_ptr<base::DictionaryValue> args)
  : ApiBinding("session", id)
{

  bool off_the_record = true;
  args->GetBoolean("off_the_record", &off_the_record);

  std::string path = "http://google.com";
  args->GetString("path", &path);

  bool dummy_cookie_store = false;
  args->GetBoolean("dummy_cookie_store", &dummy_cookie_store);

  session_.reset(new ExoSession(off_the_record, 
                                path, 
                                dummy_cookie_store));
}

ExoSessionBinding::~ExoSessionBinding()
{
  LOG(INFO) << "ExoSessionBinding Destructor";
  session_.reset();
}


  void
ExoSessionBinding::LocalCall(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const ApiHandler::ActionCallback& callback)
{
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "CALL " << method;

  callback.Run(std::string(""), 
               scoped_ptr<base::Value>(res).Pass());
}


} // namespace exo_shell
