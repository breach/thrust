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
// This represents a web frame within the ExoBrowser. It is layed out using the
// ExoBrowser fixed layout. These frames are created from Javascript and are
// therefore aware of their associated Javascript wrapper.
//
// The ExoFrame lives on the BrowserThread::UI thread, and should PostTask on
// the NodeJS thread whenever it wants to communicate with its JS Wrapper
class ExoFrame : public content::NotificationObserver {
public:
  /****************************************************************************/
  /*                         PUBLIC INTERFACE                                 */
  /****************************************************************************/
  // ### ~ExoFrame
  ~ExoFrame();

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

  // ### SetSize
  // Sets the frame size
  void SetSize(gfx::Size& size);

  // ### SetPosition
  // Sets the frame position
  void SetPosition(gfx::Point& position);


  // ### size
  // Retrieves the frame size
  gfx::Size size() { return PlatformSize(); }

  // ### position
  // Retrieves the frame position
  gfx::Point position() { return PlatformPosition(); }

  // ### name
  // Returns the ExoFrame name
  const std::string& name() { return name_; }


  // ### parent
  // Returns the ExoFrame's parent ExoBrowser
  ExoBrowser* parent() { return parent_; }

private:
  /****************************************************************************/
  /*                           PRIVATE INTERFACE                              */
  /****************************************************************************/
  // ### ExoFrame
  // Constructor used when a new WebContents has already been created for us.
  // (generally a popup).
  explicit ExoFrame(std::string& name,
                    content::WebContents* web_contents,
                    ExoFrameWrap* wrapper);

  // ### ExoFrame
  // Constructor used to create a new ExoFrame with a fresh WebContents object.
  // A call to LoadURL should be performed right after.
  explicit ExoFrame(std::string& name,
                    ExoFrameWrap* wrapper);

  // ### SetParent
  // Sets the parent ExoBrowser
  void SetParent(ExoBrowser* parent);

  /****************************************************************************/
  /*                            PLATFORM INTERFACE                            */
  /****************************************************************************/
  // All the methods that begin with Platform need to be implemented by the
  // platform specific Browser implementation.
  
  // ### PlatformSetSize
  // Let the platform set the size of the WebContents view within its parent
  // ExoBrowser main view.
  void PlatformSetSize(gfx::Size& size);

  // ### PlatformSetPosition
  // Let the platform set the position of the WebContents view within its parent
  // ExoBrowser main view.
  void PlatformSetPosition(gfx::Point& position);

  // ### PlatformSize
  // Retrieves the size of the WebContents view
  gfx::Size PlatformSize();

  // ### PlatformPosition
  // Retrieves the position of the WebContents view within its parent view
  gfx::Point PlatformPosition();

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

  ExoBrowser*                      parent_;
  ExoFrameWrap*                    wrapper_;

  scoped_ptr<content::WebContents> web_contents_;
  content::NotificationRegistrar   registrar_;

  friend class ExoBrowser;
};

} // namespace breach

#endif // BREACH_BROWSER_UI_EXO_FRAME_H_
