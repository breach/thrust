// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.
//
#include "src/browser/session/exo_session_visitedlink_store.h"

#include "url/gurl.h"
#include "content/public/browser/browser_thread.h"

#include "src/browser/session/exo_session.h"
#include "src/browser/visitedlink/visitedlink_master.h"

using namespace content;

namespace exo_browser {

ExoSessionVisitedLinkStore::ExoSessionVisitedLinkStore(
    ExoSession* parent)
: parent_(parent),
  visitedlink_master_(new visitedlink::VisitedLinkMaster(
        parent, this, !parent->IsOffTheRecord()))
{
}

ExoSessionVisitedLinkStore::~ExoSessionVisitedLinkStore()
{
}

bool
ExoSessionVisitedLinkStore::Init()
{
  return visitedlink_master_->Init();
}

void
ExoSessionVisitedLinkStore::Add(
    const std::string& url)
{
  if(!parent_->IsOffTheRecord()) {
    visitedlink_master_->AddURL(GURL(url));
  }
}

void
ExoSessionVisitedLinkStore::Clear()
{
  visitedlink_master_->DeleteAllURLs();
}


void 
ExoSessionVisitedLinkStore::RebuildTable(
    const scoped_refptr<URLEnumerator>& enumerator)
{
  /* We return no URL as the master here. The master takes care of persisting */
  /* the visited links, and this API is used in case of failure or when off   */
  /* the record.                                                              */
  enumerator->OnComplete(true);
}

}  // namespace exo_browser
