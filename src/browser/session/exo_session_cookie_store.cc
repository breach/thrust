// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/session/exo_session_cookie_store.h"

#include "content/public/browser/browser_thread.h"

#include "src/browser/session/exo_session.h"

using namespace content;

namespace exo_browser {
  
ExoSessionCookieStore::ExoSessionCookieStore(
    ExoSession* parent)
: parent_(parent),
  op_count_(0) 
{
}

ExoSessionCookieStore::~ExoSessionCookieStore()
{
  LOG(INFO) << "ExoSesionCookieStore Destructor";
}

void 
ExoSessionCookieStore::Load(
    const LoadedCallback& loaded_callback)
{
  LOG(INFO) << "Load";
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoSessionWrap::CallCookiesLoad, 
                   parent_->wrapper_, loaded_callback));

  }
  */
}

void 
ExoSessionCookieStore::LoadCookiesForKey(
    const std::string& key,
    const LoadedCallback& loaded_callback)
{
  LOG(INFO) << "LoadCookiesForKey: '" << key << "'";
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoSessionWrap::CallCookiesLoadForKey, 
                   parent_->wrapper_, key, loaded_callback));

  }
  */
}

void 
ExoSessionCookieStore::Flush(
    const base::Closure& callback)
{
  LOG(INFO) << "Flush";
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoSessionWrap::CallCookiesFlush, 
                   parent_->wrapper_, callback));

  }
  */
}


void 
ExoSessionCookieStore::AddCookie(
    const net::CanonicalCookie& cc)
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoSessionWrap::DispatchCookiesAdd, 
                   parent_->wrapper_, cc, op_count_++));

  }
  */
}

void 
ExoSessionCookieStore::UpdateCookieAccessTime(
    const net::CanonicalCookie& cc)
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoSessionWrap::DispatchCookiesUpdateAccessTime, 
                   parent_->wrapper_, cc, op_count_++));

  }
  */
}

void 
ExoSessionCookieStore::DeleteCookie(
    const net::CanonicalCookie& cc)
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoSessionWrap::DispatchCookiesDelete, 
                   parent_->wrapper_, cc, op_count_++));

  }
  */
}

void 
ExoSessionCookieStore::SetForceKeepSessionState()
{
  /*
  if(parent_ && parent_->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoSessionWrap::DispatchCookiesForceKeepSessionState, 
                   parent_->wrapper_));

  }
  */
}

}  // namespace exo_browser
