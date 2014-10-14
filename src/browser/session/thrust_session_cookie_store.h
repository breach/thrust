// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_SESSION_THRUST_SESSION_COOKIE_STORE_H_
#define THRUST_SHELL_BROWSER_SESSION_THRUST_SESSION_COOKIE_STORE_H_

#include "net/cookies/cookie_monster.h"

namespace thrust_shell {

class ThrustSession;

// ### ThrustSessionCookieStore
//
// The ThrustSessionCookieStore is a glue between the CookieMonster Delegate and
// PersistentCookieStore interface and the ThrustSession JS API. It calls into
// JS whenever it receives a call from the CookieMonster of which it is both
// the Delegate and the PersistentCookieStore.
//
// The ThrustSessionCookieStore is RefCounted and therefore owned by the URLRequest
// context that uses it. We also keep track of the parent ThrustSession to be able
// to not call into its wrapper if it's been deleted (which should not happen in
// theory)
class ThrustSessionCookieStore : public net::CookieMonster::PersistentCookieStore {

typedef net::CookieMonster::PersistentCookieStore::LoadedCallback
  LoadedCallback;

public:
  // ### ThrustSessionCookieStore
  // We keep a pointer to the parent ThrustSession to call into the JS API
  ThrustSessionCookieStore(ThrustSession* parent, bool dummy = false);

  /****************************************************************************/
  /* COOKIE MONSTER PERSISTENT STORE IMPLEMENTATION                           */
  /****************************************************************************/
  virtual void Load(const LoadedCallback& loaded_callback) OVERRIDE;

  virtual void LoadCookiesForKey(
      const std::string& key,
      const LoadedCallback& loaded_callback) OVERRIDE;

  virtual void Flush(const base::Closure& callback) OVERRIDE;

  virtual void AddCookie(const net::CanonicalCookie& cc) OVERRIDE;
  virtual void UpdateCookieAccessTime(const net::CanonicalCookie& cc) OVERRIDE;
  virtual void DeleteCookie(const net::CanonicalCookie& cc) OVERRIDE;

  virtual void SetForceKeepSessionState() OVERRIDE;

private:
  virtual ~ThrustSessionCookieStore();

  ThrustSession*         parent_;
  bool                   dummy_;

  unsigned int           op_count_;

  friend class ThrustSession;

  DISALLOW_COPY_AND_ASSIGN(ThrustSessionCookieStore);
};
  
} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_SESSION_THRUST_SESSION_COOKIE_STORE_H_
