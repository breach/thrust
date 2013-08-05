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
// ExoBrowser fixed layout. These frames are added from Javascript and it is
// therefore aware of its associated Javascript wrapper.
//
// The ExoFrame lives on the BrowserThread::UI thread, and should PostTask on
// the NodeJS thread whenever it wants to communicate with its JS Wrapper
class ExoFrame {
public:
  /****************************************************************************/
  /*                         STATIC INTERFACE                                 */
  /****************************************************************************/
  // ### Initialize
  // Runs one-time initialization at application startup.
  static void Initialize();

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

  // ### GoBack
  // Go Back in the ExoFrame browsing history.
  void GoBack();

  // ### GoForward
  // Go Forward in the ExoFrame browsing history.
  void GoForward();

  // ### Reload
  // Reloads the ExoFrame content.
  void Reload();

  // ### Stop
  // Stop loading the ExoFrame content.
  void Stop();


  // ### size
  // Retrieves the frame size
  gfx::Size size();

  // ### position
  // Retrieves the frame position
  gfx::Point position();

  // ### name
  // Returns the ExoFrame name
  const std::string& name() { return name_ };

  // ### parent
  // Returns the ExoFrame's parent ExoBrowser
  ExoBrowser* parent() { return parent_ };

private:
  /****************************************************************************/
  /*                           PRIVATE INTERFACE                              */
  /****************************************************************************/
  explicit ExoFrame(std::string& name,
                    ExoBrowser* parent,
                    ExoFrameWrap* wrapper,
                    content::WebContents* web_contents);

  /****************************************************************************/
  /*                            PLATFORM INTERFACE                            */
  /****************************************************************************/
  // All the methods that begin with Platform need to be implemented by the
  // platform specific Browser implementation.
  
  // ### PlatformSetSize
  // Let the platform set the size of the WebContents view within its parent
  // ExoBrowser main view.
  void PlatformSetSize();

  // ### PlatformSetSize
  // Let the platform set the position of the WebContents view within its parent
  // ExoBrowser main view.
  void PlatformSetSize();

  // ### GetWebContentsView
  gfx::NativeView GetContentView();

  /****************************************************************************/
  /*                               MEMBERS                                    */
  /****************************************************************************/
  std::string                 name_;
  ExoBrowser*                 parent_;
  ExoBrowserWrap*             wrapper_;

  // TODO(spolu): Add wrapper_ member

  friend class ExoBrowser;
};

} // namespace breach

#endif // BREACH_BROWSER_UI_EXO_FRAME_H_
