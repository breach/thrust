// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UI_WEB_CONTENTS_VIEW_DELEGATE_CREATOR_H_
#define EXO_BROWSER_BROWSER_UI_WEB_CONTENTS_VIEW_DELEGATE_CREATOR_H_

namespace content {
class WebContents;
class WebContentsViewDelegate;
} // namespace content

namespace exo_browser {

content::WebContentsViewDelegate* CreateBreachWebContentsViewDelegate(
    content::WebContents* web_contents);

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_UI_WEB_CONTENTS_VIEW_DELEGATE_CREATOR_H_
