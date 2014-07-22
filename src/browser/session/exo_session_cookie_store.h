// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_COOKIE_STORE_H_
#define EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_COOKIE_STORE_H_

#include "net/cookies/cookie_monster.h"

namespace exo_browser {

class ExoSession;

// ### ExoSessionCookieStore
//
// The ExoSessionCookieStore is a glue between the CookieMonster Delegate and
// PersistentCookieStore interface and the ExoSession JS API. It calls into
// JS whenever it receives a call from the CookieMonster of which it is both
// the Delegate and the PersistentCookieStore.
//
// The ExoSessionCookieStore is RefCounted and therefore owned by the URLRequest
// context that uses it. We also keep track of the parent ExoSession to be able
// to not call into its wrapper if it's been deleted (which should not happen in
// theory)
class ExoSessionCookieStore : public net::CookieMonster::PersistentCookieStore {

typedef net::CookieMonster::PersistentCookieStore::LoadedCallback
  LoadedCallback;

public:
  // ### ExoSessionCookieStore
  // We keep a pointer to the parent ExoSession to call into the JS API
  ExoSessionCookieStore(ExoSession* parent);

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
  virtual ~ExoSessionCookieStore();

  ExoSession*         parent_;
  unsigned int        op_count_;

  friend class ExoSession;

  DISALLOW_COPY_AND_ASSIGN(ExoSessionCookieStore);
};
  
} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_COOKIE_STORE_H_
