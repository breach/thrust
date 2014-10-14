// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/session/thrust_session_cookie_store.h"

#include "content/public/browser/browser_thread.h"

#include "src/browser/session/thrust_session.h"

using namespace content;

namespace thrust_shell {
  
ThrustSessionCookieStore::ThrustSessionCookieStore(
    ThrustSession* parent,
    bool dummy)
: parent_(parent),
  dummy_(dummy),
  op_count_(0) 
{
}

ThrustSessionCookieStore::~ThrustSessionCookieStore()
{
  LOG(INFO) << "ExoSesionCookieStore Destructor";
}

void 
ThrustSessionCookieStore::Load(
    const LoadedCallback& loaded_callback)
{
  LOG(INFO) << "Load";

  if(dummy_) {
    std::vector<net::CanonicalCookie*> ccs;
    content::BrowserThread::PostTask(
        content::BrowserThread::IO, FROM_HERE,
        base::Bind(loaded_callback, ccs));
  }
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ThrustSessionWrap::CallCookiesLoad, 
                   parent_->wrapper_, loaded_callback));

  }
  */
}

void 
ThrustSessionCookieStore::LoadCookiesForKey(
    const std::string& key,
    const LoadedCallback& loaded_callback)
{
  LOG(INFO) << "LoadCookiesForKey: '" << key << "'";

  if(dummy_) {
    std::vector<net::CanonicalCookie*> ccs;
    content::BrowserThread::PostTask(
        content::BrowserThread::IO, FROM_HERE,
        base::Bind(loaded_callback, ccs));
  }
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ThrustSessionWrap::CallCookiesLoadForKey, 
                   parent_->wrapper_, key, loaded_callback));

  }
  */
}

void 
ThrustSessionCookieStore::Flush(
    const base::Closure& callback)
{
  LOG(INFO) << "Flush";

  if(dummy_) {
    content::BrowserThread::PostTask(
        content::BrowserThread::IO, FROM_HERE, callback);
  }
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ThrustSessionWrap::CallCookiesFlush, 
                   parent_->wrapper_, callback));

  }
  */
}


void 
ThrustSessionCookieStore::AddCookie(
    const net::CanonicalCookie& cc)
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ThrustSessionWrap::DispatchCookiesAdd, 
                   parent_->wrapper_, cc, op_count_++));

  }
  */
}

void 
ThrustSessionCookieStore::UpdateCookieAccessTime(
    const net::CanonicalCookie& cc)
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ThrustSessionWrap::DispatchCookiesUpdateAccessTime, 
                   parent_->wrapper_, cc, op_count_++));

  }
  */
}

void 
ThrustSessionCookieStore::DeleteCookie(
    const net::CanonicalCookie& cc)
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ThrustSessionWrap::DispatchCookiesDelete, 
                   parent_->wrapper_, cc, op_count_++));

  }
  */
}

void 
ThrustSessionCookieStore::SetForceKeepSessionState()
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ThrustSessionWrap::DispatchCookiesForceKeepSessionState, 
                   parent_->wrapper_));

  }
  */
}

}  // namespace thrust_shell
