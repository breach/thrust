// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_VISITEDLINK_STORE_H_
#define EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_VISITEDLINK_STORE_H_

#include <string>

#include "base/memory/scoped_ptr.h"
#include "src/browser/visitedlink/visitedlink_delegate.h"

namespace visitedlink {
class VisitedLinkMaster;
}

namespace exo_browser {

class ExoSession;

// ### ExoSessionVisitedLinkStore
//
// The ExoSessionVisitedLinkStore is a wrapper around the VisitedLinkMaster.
// It is a VisitedLinkDelegate (but returns not visited link) as the master
// already takes care of storing links on disk and read from it if the browser
// context is not off the record.
// TODO(spolu): Later on we may want to expose the delgate API to JS but this
//              is low priority as it does not bring that much value for now.
class ExoSessionVisitedLinkStore 
  : public visitedlink::VisitedLinkDelegate,
    public base::RefCountedThreadSafe<ExoSessionVisitedLinkStore> {
public:
  // ### ExoSessionVisitedLinkStore
  // We keep a pointer to the parent ExoSession to call into the JS API
  ExoSessionVisitedLinkStore(ExoSession* parent);

  // ### Init
  // Initialiazes the VisitedLinkStore (underlying VisitedLinkMaster 
  // initialization)
  bool Init();

  // ### Add
  // Adds an URL to the VisitedLink Store (underlying visitedlink_master)
  // ```
  // @url {string} the URL to add
  // ```
  void Add(const std::string& url);

  // ### Clear
  // Clears all VisitedLinks and destroys the file system storage as well
  void Clear();

  /****************************************************************************/
  /* VISITED LINK DELEGATE IMPLEMENTATION                                     */
  /****************************************************************************/
  virtual void RebuildTable(
      const scoped_refptr<URLEnumerator>& enumerator) OVERRIDE;

private:
  virtual ~ExoSessionVisitedLinkStore();

  ExoSession*                                parent_;
  scoped_ptr<visitedlink::VisitedLinkMaster> visitedlink_master_;

  friend class ExoSession;
  friend class base::RefCountedThreadSafe<ExoSessionVisitedLinkStore>;

  DISALLOW_COPY_AND_ASSIGN(ExoSessionVisitedLinkStore);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_SESSION_EXO_SESSION_VISITEDLINK_STORE_H_
