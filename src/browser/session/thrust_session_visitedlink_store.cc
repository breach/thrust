// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.
//
#include "src/browser/session/thrust_session_visitedlink_store.h"

#include "url/gurl.h"
#include "content/public/browser/browser_thread.h"

#include "src/browser/session/thrust_session.h"
#include "src/browser/visitedlink/visitedlink_master.h"

using namespace content;

namespace thrust_shell {

ThrustSessionVisitedLinkStore::ThrustSessionVisitedLinkStore(
    ThrustSession* parent)
: parent_(parent),
  visitedlink_master_(new visitedlink::VisitedLinkMaster(
        parent, this, !parent->IsOffTheRecord()))
{
}

ThrustSessionVisitedLinkStore::~ThrustSessionVisitedLinkStore()
{
}

bool
ThrustSessionVisitedLinkStore::Init()
{
  return visitedlink_master_->Init();
}

void
ThrustSessionVisitedLinkStore::Add(
    const std::string& url)
{
  if(!parent_->IsOffTheRecord()) {
    visitedlink_master_->AddURL(GURL(url));
  }
}

void
ThrustSessionVisitedLinkStore::Clear()
{
  visitedlink_master_->DeleteAllURLs();
}


void 
ThrustSessionVisitedLinkStore::RebuildTable(
    const scoped_refptr<URLEnumerator>& enumerator)
{
  /* We return no URL as the master here. The master takes care of persisting */
  /* the visited links, and this API is used in case of failure or when off   */
  /* the record.                                                              */
  enumerator->OnComplete(true);
}

}  // namespace thrust_shell
