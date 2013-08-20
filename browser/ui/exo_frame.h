// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef BREACH_BROWSER_UI_EXO_FRAME_H_
#define BREACH_BROWSER_UI_EXO_FRAME_H_

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"

#if defined(TOOLKIT_GTK)
#include <gtk/gtk.h>
#include "ui/base/gtk/gtk_signal.h"

typedef struct _GtkToolItem GtkToolItem;
namespace views {
class Widget;
class ViewsDelegate;
}
#endif

class GURL;

namespace content {
class BrowserContext;
class SiteInstance;
class WebContents;
}

namespace breach {

class ExoBrowser;
class ExoFrameWrap;

// ### ExoFrame
//
// This represents a web frame within the ExoBrowser. It can be used as a `page`
// (stacked within the browser) or as a control. As a consequence it does not
// have the ability to layout itself. The Size and Position is computed by the
// managing ExoBrowser.
//
// Frames are created from Javascript and are therefore aware of their 
// associated JS wrapper.
//
// The ExoFrame lives on the BrowserThread::UI thread, and should PostTask on
// the NodeJS thread whenever it wants to communicate with its JS Wrapper
class ExoFrame : public content::NotificationObserver,
                 public content::WebContentsObserver {
public:
  /****************************************************************************/
  /*                         PUBLIC INTERFACE                                 */
  /****************************************************************************/
  // ### ~ExoFrame
  virtual ~ExoFrame();

  // ### FRAME_TYPE
  // An enum representing the frame type (control or page)
  enum FRAME_TYPE {
    NOTYPE_FRAME = 0,
    CONTROL_FRAME,
    PAGE_FRAME,
    FRAME_TYPE_COUNT
  };

  // ### LoadURL
  // ```
  // @url {URL} the url to load
  // ```
  // Loads the provided url in this ExoFrame.
  void LoadURL(const GURL& url);

  // ### GoBackOrForward
  // ```
  // @offet {int} go back or forward of offset
  // ```
  // Go Back or Forward in the ExoFrame browsing history.
  void GoBackOrForward(int offset);

  // ### Reload
  // Reloads the ExoFrame content.
  void Reload();

  // ### Stop
  // Stop loading the ExoFrame content.
  void Stop();

  // ### Focus
  // Focuses the ExoFrame
  void Focus();

  // ### type
  // Returns the frame type
  FRAME_TYPE type() { return type_; }

  // ### size
  // Retrieves the frame size
  gfx::Size size() { return PlatformSize(); }

  // ### name
  // Returns the ExoFrame name
  const std::string& name() { return name_; }


  // ### parent
  // Returns the ExoFrame's parent ExoBrowser
  ExoBrowser* parent() { return parent_; }

  /****************************************************************************/
  /*                   WEBCONTENTSOBSERVER IMPLEMENTATION                     */
  /****************************************************************************/
  virtual void DidUpdateFaviconURL(
      int32 page_id,
      const std::vector<content::FaviconURL>& candidates) OVERRIDE;

  virtual void ProvisionalChangeToMainFrameUrl(
      const GURL& url,
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void DidFailLoad(
      int64 frame_id,
      const GURL& validated_url,
      bool is_main_frame,
      int error_code,
      const string16& error_description,
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void DidFinishLoad(
      int64 frame_id,
      const GURL& validated_url,
      bool is_main_frame,
      content::RenderViewHost* render_view_host) OVERRIDE;

  virtual void DidStartLoading(
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void DidStopLoading(
      content::RenderViewHost* render_view_host) OVERRIDE;


private:
  /****************************************************************************/
  /*                           PRIVATE INTERFACE                              */
  /****************************************************************************/
  // ### ExoFrame
  // Constructor used when a new WebContents has already been created for us.
  // (generally a popup).
  explicit ExoFrame(const std::string& name,
                    content::WebContents* web_contents,
                    ExoFrameWrap* wrapper);

  // ### ExoFrame
  // Constructor used to create a new ExoFrame with a fresh WebContents object.
  // A call to LoadURL should be performed right after.
  explicit ExoFrame(const std::string& name,
                    ExoFrameWrap* wrapper);

  // ### SetParent
  // Sets the parent ExoBrowser
  void SetParent(ExoBrowser* parent);


  // ### SetType
  // ```
  // @type {FRAME_TYPE} the frame type
  // ```
  // Sets the frame type so that it's easily `recognizable'. Should only be 
  // called by parent ExoBrowser.
  void SetType(FRAME_TYPE type);


  /****************************************************************************/
  /*                            PLATFORM INTERFACE                            */
  /****************************************************************************/
  // All the methods that begin with Platform need to be implemented by the
  // platform specific Browser implementation.
  
  // ### PlatformSize
  // Retrieves the size of the WebContents view
  gfx::Size PlatformSize();

  // ### PlatformFocus
  // Attempts to focus this widget within its window
  void PlatformFocus();

  /****************************************************************************/
  /*                  NOTIFICATION OBSERVER IMPLEMENTATION                    */
  /****************************************************************************/
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;


  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  std::string                      name_;
  FRAME_TYPE                       type_;

  ExoBrowser*                      parent_;
  ExoFrameWrap*                    wrapper_;

  scoped_ptr<content::WebContents> web_contents_;
  content::NotificationRegistrar   registrar_;

  friend class ExoBrowser;
  friend class ExoFrameWrap;
  friend class ExoBrowserWrap;
};

} // namespace breach

#endif // BREACH_BROWSER_UI_EXO_FRAME_H_
