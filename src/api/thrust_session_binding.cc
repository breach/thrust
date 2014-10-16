// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/api/thrust_session_binding.h"

#include "base/time/time.h"
#include "url/gurl.h"
#include "net/cookies/cookie_util.h"
#include "content/public/browser/browser_thread.h"

#include "src/browser/session/thrust_session.h"

namespace {

static base::DictionaryValue*
ValueFromCanonicalCookie(
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
CanonicalCookieFromValue(
    base::DictionaryValue* cookie_v)
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

  bool cookie_store = false;
  args->GetBoolean("cookie_store", &cookie_store);

  session_.reset(new ThrustSession(this,
                                   off_the_record, 
                                   path, 
                                   !cookie_store));
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
  else if(method.compare("visitedlink_add") == 0) {
    std::string url = "";
    args->GetString("url", &url);
    session_->GetVisitedLinkStore()->Add(url);
  }
  else if(method.compare("visitedlink_clear") == 0) {
    session_->GetVisitedLinkStore()->Clear();
  }
  else {
    err = "exo_session_binding:method_not_found";
  }

  callback.Run(err, scoped_ptr<base::DictionaryValue>(res).Pass());
}

void
ThrustSessionBinding::CookiesLoadCallback(
    const LoadedCallback& loaded_callback,
    const std::string& error,
    scoped_ptr<base::DictionaryValue> result)
{
  /* Runs on UI thread. */
  std::vector<net::CanonicalCookie*> ccs;

  base::ListValue* cookies;
  if(result->GetList("cookies", &cookies)) {
    for(size_t i = 0; i < cookies->GetSize(); i++) {
      base::DictionaryValue* cookie;
      if(cookies->GetDictionary(i, &cookie)) {
        ccs.push_back(CanonicalCookieFromValue(cookie));
      }
    }
  }

  LOG(INFO) << "COOKIE LOAD CALLBACK " << error;
  LOG(INFO) << ccs.size();

  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(loaded_callback, ccs));
}

void
ThrustSessionBinding::CookiesLoad(
    const LoadedCallback& loaded_callback)
{
  /* Runs on UI thread. */
  base::DictionaryValue* args = new base::DictionaryValue;

  this->InvokeRemoteMethod("cookies_load", 
                           scoped_ptr<base::DictionaryValue>(args).Pass(),
                           base::Bind(&ThrustSessionBinding::CookiesLoadCallback, 
                                      this, loaded_callback));
}

void 
ThrustSessionBinding::CookiesLoadForKey(
    const std::string& key,
    const LoadedCallback& loaded_callback)
{
  /* Runs on UI thread. */
  base::DictionaryValue* args = new base::DictionaryValue;
  args->SetString("key", key);

  this->InvokeRemoteMethod("cookies_load_for_key", 
                           scoped_ptr<base::DictionaryValue>(args).Pass(),
                           base::Bind(&ThrustSessionBinding::CookiesLoadCallback, 
                                      this, loaded_callback));
}

void
ThrustSessionBinding::CookiesFlushCallback(
    const base::Closure& callback,
    const std::string& error,
    scoped_ptr<base::DictionaryValue> result)
{
  /* Runs on UI thread. */
  if(error.size() == 0) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO, FROM_HERE, callback);
  }
  /* Otherwise if an error occured we don't call the callback. */
}

void
ThrustSessionBinding::CookiesFlush(
    const base::Closure& callback)
{
  /* Runs on UI thread. */
  base::DictionaryValue* args = new base::DictionaryValue;

  this->InvokeRemoteMethod("cookies_flush", 
                           scoped_ptr<base::DictionaryValue>(args).Pass(),
                           base::Bind(&ThrustSessionBinding::CookiesFlushCallback, 
                                      this, callback));
}

void
ThrustSessionBinding::CookiesAdd(
    const net::CanonicalCookie& cc,
    unsigned int op_count)
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  evt->Set("cookie", ValueFromCanonicalCookie(cc));
  evt->SetInteger("op_count", op_count);

  this->EmitEvent("cookies_add", scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void
ThrustSessionBinding::CookiesUpdateAccessTime(
    const net::CanonicalCookie& cc,
    unsigned int op_count)
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  evt->Set("cookie", ValueFromCanonicalCookie(cc));
  evt->SetInteger("op_count", op_count);

  this->EmitEvent("cookies_update_access_time", 
                  scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void
ThrustSessionBinding::CookiesDelete(
    const net::CanonicalCookie& cc,
    unsigned int op_count)
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  evt->Set("cookie", ValueFromCanonicalCookie(cc));
  evt->SetInteger("op_count", op_count);

  this->EmitEvent("cookies_delete", 
                  scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void
ThrustSessionBinding::CookiesForceKeepSessionState()
{
  base::DictionaryValue* evt = new base::DictionaryValue;

  this->EmitEvent("cookies_force_keep_session_state", 
                  scoped_ptr<base::DictionaryValue>(evt).Pass());
}



ThrustSession*
ThrustSessionBinding::GetSession() {
  return session_.get();
}


} // namespace thrust_shell
