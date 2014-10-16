// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_API_THRUST_SESSION_BINDING_H_
#define THRUST_SHELL_API_THRUST_SESSION_BINDING_H_

#include "base/callback.h"

#include "src/api/api_binding.h"
#include "net/cookies/cookie_monster.h"

namespace thrust_shell {

class ThrustSession;

class ThrustSessionBinding : public APIBinding {
public:

  typedef net::CookieMonster::PersistentCookieStore::LoadedCallback
        LoadedCallback;

  /****************************************************************************/
  /* API BINDING INTERFACE */
  /****************************************************************************/
  ThrustSessionBinding(const unsigned int id, 
                    scoped_ptr<base::DictionaryValue> args);
  ~ThrustSessionBinding();

  virtual void CallLocalMethod(
      const std::string& method, 
      scoped_ptr<base::DictionaryValue> args, 
      const API::MethodCallback& callback) OVERRIDE;

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  void CookiesLoadCallback(const LoadedCallback& loaded_callback,
                           const std::string& error,
                           scoped_ptr<base::DictionaryValue> result);
  void CookiesLoad(const LoadedCallback& loaded_callback);
  void CookiesLoadForKey(const std::string& key,
                         const LoadedCallback& loaded_callback);

  void CookiesFlushCallback(const base::Closure& callback,
                            const std::string& error,
                            scoped_ptr<base::DictionaryValue> result);
  void CookiesFlush(const base::Closure& callback);

  void CookiesAdd(const net::CanonicalCookie& cc,
                  unsigned int op_count);
  void CookiesUpdateAccessTime(const net::CanonicalCookie& cc,
                               unsigned int op_count);
  void CookiesDelete(const net::CanonicalCookie& cc,
                     unsigned int op_count);
  void CookiesForceKeepSessionState();

  ThrustSession* GetSession();

private:
  scoped_ptr<ThrustSession> session_;
};


// ## ThrustSessionBindingFactory
//
// Factory object used to generate ThrustSession bindings
class ThrustSessionBindingFactory : public APIBindingFactory {
public:
  ThrustSessionBindingFactory();
  ~ThrustSessionBindingFactory();

  APIBinding* Create(const unsigned int id, 
                     scoped_ptr<base::DictionaryValue> args) OVERRIDE;
};

} // namespace thrust_shell
  
#endif // THRUST_SHELL_API_THRUST_SESSION_BINDING_H_
