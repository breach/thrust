// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UI_EXO_FRAME_H_
#define EXO_BROWSER_BROWSER_UI_EXO_FRAME_H_

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "content/public/common/stop_find_action.h"
#include "content/public/common/page_zoom.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"

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

namespace WebKit {
struct WebFindOptions;
}

namespace exo_browser {

class ExoSession;
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
  /* PUBLIC INTERFACE                                                         */
  /****************************************************************************/
  // ### ~ExoFrame
  virtual ~ExoFrame();

  // ### FRAME_TYPE
  //
  // An enum representing the frame type (control or page)
  enum FRAME_TYPE {
    NOTYPE_FRAME = 0,
    CONTROL_FRAME,
    PAGE_FRAME,
    FRAME_TYPE_COUNT
  };

  // ### LoadURL
  //
  // Loads the provided url in this ExoFrame.
  // ```
  // @url {URL} the url to load
  // ```
  void LoadURL(const GURL& url);

  // ### GoBackOrForward
  //
  // Go Back or Forward in the ExoFrame browsing history.
  // ```
  // @offet {int} go back or forward of offset
  // ```
  void GoBackOrForward(int offset);

  // ### Reload
  //
  // Reloads the ExoFrame content.
  void Reload();

  // ### Stop
  //
  // Stop loading the ExoFrame content.
  void Stop();

  // ### Focus
  //
  // Focuses the ExoFrame
  void Focus();

  // ### Find
  //
  // Searches for a string in the frame. We stay close to the Content API for 
  // now
  void Find(int request_id, const string16& search_text,
            const WebKit::WebFindOptions& options);
  // ### StopFinding
  //
  // Stop the finding of a string
  void StopFinding(content::StopFindAction action);

  // ### CaptureFrame
  //
  // Captures the current ExoFrame and returns the image as a PNG base64 string 
  // ```
  // @callback {Callback}
  // ```
  void CaptureFrame(
      const base::Callback<void(bool, const std::string&)>& callback);

  // ### Zoom
  //
  // Zoom the page in/out/reset
  // ```
  // @zoom {PageZoom} in/out/reset
  // ```
  void Zoom(const content::PageZoom zoom);
  
  // ### GetZoomLevel
  //
  // Retrieves the current Zoom Level
  double ZoomLevel() const;

  // ### type
  //
  // Returns the frame type
  FRAME_TYPE type() const { return type_; }

  // ### size
  //
  // Retrieves the frame size
  gfx::Size size() { return PlatformSize(); }

  // ### name
  //
  // Returns the ExoFrame name
  const std::string& name() const { return name_; }


  // ### parent
  //
  // Returns the ExoFrame's parent ExoBrowser
  ExoBrowser* parent() const { return parent_; }


  /****************************************************************************/
  /* WEBCONTENTSOBSERVER IMPLEMENTATION                                       */
  /****************************************************************************/
  virtual void DidUpdateFaviconURL(
      int32 page_id,
      const std::vector<content::FaviconURL>& candidates) OVERRIDE;

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
  /* PRIVATE INTERFACE                                                        */
  /****************************************************************************/
  // ### ExoFrame
  // Constructor used when a new WebContents has already been created for us.
  // (generally a popup). No ExoSession is required here as the WebContents is
  // already created.
  explicit ExoFrame(const std::string& name,
                    content::WebContents* web_contents,
                    ExoFrameWrap* wrapper = NULL);

  // ### ExoFrame
  // Constructor used to create a new ExoFrame with a fresh WebContents object.
  // A call to LoadURL should be performed right after. An ExoSession is
  // required here as an underlying WebContents is created.
  explicit ExoFrame(const std::string& name,
                    ExoSession* session,
                    ExoFrameWrap* wrapper = NULL);

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
  /* PLATFORM INTERFACE                                                       */
  /****************************************************************************/
  // All the methods that begin with Platform need to be implemented by the
  // platform specific Browser implementation.
  
  // ### PlatformSize
  // Retrieves the size of the WebContents view
  gfx::Size PlatformSize();

  /****************************************************************************/
  /* NOTIFICATION OBSERVER IMPLEMENTATION                                     */
  /****************************************************************************/
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  /****************************************************************************/
  /* CAPTUREFRAME ASYNCHRONOUS IMPLEMENTATION                                 */
  /****************************************************************************/
  // ### CaptureImpl
  //
  // Class used to be passed around across async call to capture frame
  class CaptureFrameImpl 
    : public base::RefCountedThreadSafe<CaptureFrameImpl> {

  public:
    CaptureFrameImpl(
        ExoFrame* parent,
        const base::Callback<void(bool, const std::string&)>& callback_);

    void Run();

  private:
    ~CaptureFrameImpl();

    void CopyFromBackingStoreComplete(
        bool succeeded,
        const SkBitmap& bitmap);
    void Finish(
        bool succeeded,
        const SkBitmap& screen_capture);

    ExoFrame*                                      parent_;
    base::Callback<void(bool, const std::string&)> callback_;

    friend class base::RefCountedThreadSafe<CaptureFrameImpl>;
  };


  /****************************************************************************/
  /* MEMBERS                                                                  */
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

  DISALLOW_COPY_AND_ASSIGN(ExoFrame);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_UI_EXO_FRAME_H_
