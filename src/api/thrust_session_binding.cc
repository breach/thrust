// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/api/thrust_session_binding.h"

#include "base/time/time.h"
#include "url/gurl.h"
#include "net/cookies/cookie_util.h"

#include "src/browser/session/thrust_session.h"

namespace {

static base::DictionaryValue*
ObjectFromCanonicalCookie(
    const net::CanonicalCookie& cc)
{
  base::DictionaryValue* cookie_v = new base::DictionaryValue;

  GURL url(cc.Source());
  std::string domain_key(net::cookie_util::GetEffectiveDomain(url.scheme(),
                                                              url.host()));

  cookie_v->SetString("domain_key", domain_key);
  cookie_v->SetString("source", cc.Source());
  cookie_v->SetString("name", cc.Name());
  cookie_v->SetString("value", cc.Value());
  cookie_v->SetString("domain", cc.Domain());
  cookie_v->SetString("path", cc.Path());

  cookie_v->SetInteger("creation", cc.CreationDate().ToInternalValue());
  cookie_v->SetInteger("expiry", cc.ExpiryDate().ToInternalValue());
  cookie_v->SetInteger("last_access", cc.LastAccessDate().ToInternalValue());

  cookie_v->SetInteger("secure", cc.IsSecure());
  cookie_v->SetBoolean("http_only", cc.IsHttpOnly());

  cookie_v->SetInteger("prority", cc.Priority());

  return cookie_v;
}

net::CanonicalCookie*
CanonicalCookieFromObject(
    scoped_ptr<base::DictionaryValue> cookie_v)
{
  std::string source = "";
  std::string name = "";
  std::string value = "";
  std::string domain = "";
  std::string path = "";
  int creation = 0;
  int expiry = 0;
  int last_access = 0;
  bool secure = false;
  bool http_only = false;
  int priority = 0;
  
  cookie_v->GetString("source", &source);
  cookie_v->GetString("name", &name);
  cookie_v->GetString("value", &value);
  cookie_v->GetString("domain", &domain);
  cookie_v->GetString("path", &path);

  cookie_v->GetInteger("creation", &creation);
  cookie_v->GetInteger("expiry", &expiry);
  cookie_v->GetInteger("last_access", &last_access);

  cookie_v->GetBoolean("secure", &secure);
  cookie_v->GetBoolean("http_only", &http_only);

  cookie_v->GetInteger("priority", &priority);

  net::CanonicalCookie* cc = new net::CanonicalCookie(
      GURL(source),
      name,
      value,
      domain,
      path,
      base::Time::FromInternalValue(creation),
      base::Time::FromInternalValue(expiry),
      base::Time::FromInternalValue(last_access),
      secure,
      http_only,
      (net::CookiePriority)priority);

  return cc;
}

}

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
  LOG(INFO) << "ThrustSessionBinding Constructor [" << this << "] " << id_;

  bool off_the_record = true;
  args->GetBoolean("off_the_record", &off_the_record);

  std::string path = "dummy_session";
  args->GetString("path", &path);

  bool dummy_cookie_store = true;
  args->GetBoolean("dummy_cookie_store", &dummy_cookie_store);

  session_.reset(new ThrustSession(this,
                                   off_the_record, 
                                   path, 
                                   dummy_cookie_store));
  session_->Initialize();
}

ThrustSessionBinding::~ThrustSessionBinding()
{
  LOG(INFO) << "ThrustSessionBinding Destructor [" << this << "] " << id_;
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

void
ThrustSessionBinding::CookiesLoad(
    const LoadedCallback& loaded_callback)
{
  base::DictionaryValue* args = new base::DictionaryValue;

  this->CallRemoteMethod("cookies_load", 
                         scoped_ptr<base::DictionaryValue>(args).Pass(),
                         base::Bind(&ThrustSessionBinding::CookiesLoadCallback, 
                                    this, loaded_callback));
}

void
ThrustSessionBinding::CookiesLoadCallback(
    const LoadedCallback& loaded_callback,
    const std::string& error,
    scoped_ptr<base::Value> result)
{
  LOG(INFO) << "COOKIE LOAD CALLBACK " << error;
}



ThrustSession*
ThrustSessionBinding::GetSession() {
  return session_.get();
}


} // namespace thrust_shell
