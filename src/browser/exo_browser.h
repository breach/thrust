// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UI_EXO_BROWSER_H_
#define EXO_BROWSER_BROWSER_UI_EXO_BROWSER_H_

#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
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
#elif defined(OS_MACOSX)
struct CGContext;
#ifdef __OBJC__
@class NSLayoutConstraint;
#else
class NSLayoutConstraint;
#endif // __OBJC__
#endif

class GURL;

namespace content {
class BrowserContext;
class SiteInstance;
class WebContents;
struct NativeWebKeyboardEvent;
struct FileChooserParams;
}

namespace exo_browser {

class ExoBrowserDevToolsFrontend;
class ExoBrowserJavaScriptDialogManager;

class ExoBrowserWrap;
class ExoFrameWrap;

class ExoFrame;


// ### ExoBrowser
// 
// This represents an ExoBrowser window. The ExoBrowser exposes to 
// Javascript the ability to add Pages (stacked, one visible at a time) and
// controls (pre-defined paramatrizable layout).
//
// Pages and Controls are both ExoFrames, which are simple wrapper around
// WebContents that are associated with a JS object.
//
// The pages frames are stacked with one visible at all time
//
//                         +--------------+
//                         |+--------------+
//                         ||+--------------+
//                         |||XXXXXXXXXXXXXX|
//                         |||XXXXXXXXXXXXXX|
//                         |||XXX PAGES XXXX|
//                         +||XXXXXXXXXXXXXX|
//                          +|XXXXXXXXXXXXXX|
//                           +--------------+
// 
// The control frames can be of only a limited set of predefined type. Each type
// correspond to a position on screen:
//
//                                   TOP
//                +------+-------------------------+---+
//                |      +-------------------------+   |
//                |      |XXXXXXXXXXXXXXXXXXXXXXXXX|   |
//                |      |XXXXXXXXX PAGES XXXXXXXXX|   |
//          LEFT  |      |XXXXXXXXXXXXXXXXXXXXXXXXX|   |  RIGHT
//                |      +-------------------------+   |
//                |      |                         |   |
//                |      |                         |   |
//                +------+-------------------------+---+
//                                 BOTTOM
//
// As depicted in the diagram above, each control has exactly one dimension
// (width for LEFT, RIGHT; height for TOP, BOTTOM) that can be programatically
// set and updated.
//
// The ExoBrowser initialization always come from Javascript. It is aware of its 
// associated JS wrapper (used to dispatch callbacks).
//
// The ExoBrowser lives on the BrowserThread::UI thread, and should PostTask on
// the NodeJS thread whenever it wants to communicate with its JS Wrapper
class ExoBrowser : public content::WebContentsDelegate {
public:
  static const int kDefaultWindowWidth;
  static const int kDefaultWindowHeight;
  
  /****************************************************************************/
  /* STATIC INTERFACE */
  /****************************************************************************/
  // ### Initialize
  //
  // Runs one-time initialization at application startup.
  static void Initialize();

  // ### CreateNew
  //
  // Creates a new empty ExoBrowser window.
  // ```
  // @wrapper   {ExoBrowserWrap} the wrapper associated with this ExoBrowser
  // @size      {Size} the initial size of the window
  // @icon_path {string} icon_path (no effect on OSX)
  // ```
  static ExoBrowser* CreateNew(ExoBrowserWrap* wrapper,
                               const gfx::Size& size,
                               const std::string& icon_path);

  // ### instances
  //
  // Getter for all the currently working ExoBrowser instances.
  static std::vector<ExoBrowser*>& instances() { return s_instances; }

  // ### KillAll
  //
  // Kills all running instances and returns.
  static void KillAll();

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  // ### ~ExoBrowser
  virtual ~ExoBrowser();


  // ### CONTROL_TYPE
  // An enum specifiying the different control types. Any new control should be
  // placed above CONTROL_TYPE_COUNT
  enum CONTROL_TYPE {
    NOTYPE_CONTROL = 0,
    TOP_CONTROL,
    BOTTOM_CONTROL,
    LEFT_CONTROL,
    RIGHT_CONTROL,
    CONTROL_TYPE_COUNT
  };

  // ### SetControl
  //
  // Adds a frame as control. If the control was already set, it is unset before
  // and its dimension is reinitialized to 0.
  // ```
  // @type  {CONTROL_TYPE} the control type
  // @frame {ExoFrame} the frame to add as control
  // ```
  void SetControl(CONTROL_TYPE type, ExoFrame* frame);

  // ### SetControlDimension
  //
  // Sets the control dimension size in pixel
  // ```
  // @type {CONTROL_TYPE} the control type
  // @size {int} the dimension size
  // ```
  void SetControlDimension(CONTROL_TYPE type, int size);

  // ### UnsetControl
  //
  // Unsets a control. If the control was not set, nothing is done. Otherwise
  // the associated frame is removed from this browser. The frame is not deleted
  // as its deletion is handled by its JS wrapper. The control dimension is 
  // automatically reset to 0.
  // ```
  // @type {CONTROL_TYPE} the control type
  // ```
  void UnsetControl(CONTROL_TYPE type);


  // ### AddPage
  //
  // Adds a frame to this browser as a page. The visible page is not altered by
  // this method. The frame will be refered by its name in all subsequent API
  // interactios.
  // ```
  // @frame {ExoFrame} the frame to add as a page
  // ```
  void AddPage(ExoFrame* frame);

  // ### RemovePage
  //
  // Removes the frame from this browser. The frame is not deleted.
  // ```
  // @name {std::string} the frame name
  // ```
  void RemovePage(const std::string& name);

  // ### showPage
  //
  // Make the page visible
  // ```
  // @name {std::string} the frame name
  // ```
  void ShowPage(const std::string& name);


  // ### RemoveFrame
  //
  // Removes the frame appproprietly depending on its type
  // ```
  // @name {std::string} the frame name
  // ```
  void RemoveFrame(const std::string& name);


  // ### Focus
  //
  // Focuses the ExoBrowser window
  void Focus() { PlatformFocus(); }

  // ### Maximize
  // 
  // Maximize the ExoBrowser window
  void Maximize() { PlatformMaximize(); }

  // ### SetTitle
  //
  // Sets the ExoBrowser window title
  void SetTitle(const std::string& title) { PlatformSetTitle(title); }


  // ### Kill
  // 
  // Kills the ExoBrowser and remove all of its frame. The ExoBrowser object 
  // will not be deleted on kill (as it will be reclaimed when the JS object 
  // is deleted) but it will be marked as killed as it is not usable anymore
  void Kill();


  // ### is_killed
  //
  // Returns whether the ExoBrowser is killed or not
  bool is_killed() { return is_killed_; }

  // ### WindowSize
  //
  // Retrieves the native Window size
  gfx::Size size() { return PlatformSize(); }

  // ### WindowPosition
  //
  // Retrieves the native Window position
  gfx::Point position() { return PlatformPosition(); }

  // ### window
  //
  // Retrieves the NativeWindow object
  gfx::NativeWindow window() { return window_; }

  /****************************************************************************/
  /* WEBCONTENTSDELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;

  virtual void RequestToLockMouse(content::WebContents* web_contents,
                                  bool user_gesture,
                                  bool last_unlocked_by_target) OVERRIDE;

  virtual bool PreHandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event,
      bool* is_keyboard_shortcut) OVERRIDE;
  virtual void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) OVERRIDE;

  virtual void CloseContents(content::WebContents* source) OVERRIDE;

  virtual void AddNewContents(content::WebContents* source,
                              content::WebContents* new_contents,
                              WindowOpenDisposition disposition,
                              const gfx::Rect& initial_pos,
                              bool user_gesture,
                              bool* was_blocked) OVERRIDE;

  virtual void WebContentsCreated(content::WebContents* source_contents,
                                  int64 source_frame_id,
                                  const base::string16& frame_name,
                                  const GURL& target_url,
                                  content::WebContents* new_contents) OVERRIDE;

  virtual content::JavaScriptDialogManager* 
    GetJavaScriptDialogManager() OVERRIDE;

  virtual void NavigationStateChanged(const content::WebContents* source,
                                      unsigned changed_flags) OVERRIDE;

  virtual void ActivateContents(content::WebContents* contents) OVERRIDE;
  virtual void DeactivateContents(content::WebContents* contents) OVERRIDE;

  virtual void RendererUnresponsive(content::WebContents* source) OVERRIDE;
  virtual void WorkerCrashed(content::WebContents* source) OVERRIDE;

  virtual void FindReply(content::WebContents* web_contents,
                         int request_id,
                         int number_of_matches,
                         const gfx::Rect& selection_rect,
                         int active_match_ordinal,
                         bool final_update) OVERRIDE;

  virtual void RunFileChooser(
      content::WebContents* web_contents,
      const content::FileChooserParams& params) OVERRIDE;
  virtual void EnumerateDirectory(content::WebContents* web_contents,
                                  int request_id,
                                  const base::FilePath& path) OVERRIDE;

private:
  /****************************************************************************/
  /* PRIVATE INTERFACE */
  /****************************************************************************/
  explicit ExoBrowser(ExoBrowserWrap* wrapper);

  // ### FrameForWebContents
  //
  // Retrieves within this browser, the frame associated with the provided 
  // WebContents. Returns NULL otherwise. This is usefull compared to the
  // ExoFrame static method to retrieve only frames associated with itself.
  // ```
  // @content {WebContents} a WebContents
  // ```
  ExoFrame* FrameForWebContents(const content::WebContents* web_contents);

  /****************************************************************************/
  /* STATIC PLATFORM INTERFACE */
  /****************************************************************************/
  // All the methods that begin with Platform need to be implemented by the
  // platform specific Browser implementation.

  // ### PlatformInitialize
  //
  // Helper for one time initialization of application
  static void PlatformInitialize(const gfx::Size& default_window_size);


  /****************************************************************************/
  /* PLATFORM INTERFACE */
  /****************************************************************************/
  // ### PlatformCleanup
  //
  // Called from the destructor to let each platform do any necessary cleanup.
  void PlatformCleanUp();

  // ### PlatformCreateWindow
  //
  // Creates the ExoBrowser window GUI.
  void PlatformCreateWindow(int width, int height, 
                            const std::string& icon_path);

  // ### PlatformKill
  //
  // Let each platform clean up on kill. All frames have already been removed.
  void PlatformKill();

  // ### PlatformSetTitle 
  //
  // Set the title of ExoBrowser window.
  void PlatformSetTitle(const std::string& title);


  // ### PlatformFocus
  //
  // Focuses the ExoBrowser window
  void PlatformFocus();

  // ### PlatformMaximize
  //
  // Maximizes the ExoBrowser window
  void PlatformMaximize();


  // ### PlatformAddPage
  //
  // Adds the frame web_contents view to the page view hierarchy
  void PlatformAddPage(ExoFrame *frame);

  // ### PlatformRemovePage
  //
  // Removes the frame web_contents view from the page view hierarchy
  void PlatformRemovePage(ExoFrame *frame);

  // ### PlatformShowPage
  //
  // Shows the page, hidding all other ones
  void PlatformShowPage(ExoFrame *frame);


  // ### PlatformSetControl
  //
  // Adds the frame as a control
  void PlatformSetControl(CONTROL_TYPE type, ExoFrame *frame);

  // ### PlatformSetControlDimension
  //
  // Sets the control dimenstion. Must work even if control unset.
  void PlatformSetControlDimension(CONTROL_TYPE type, int size);

  // ### PlatformUnsetControl
  //
  // Unset the designated control
  void PlatformUnsetControl(CONTROL_TYPE type, ExoFrame *frame);


  // ### PlatformSize
  //
  // Retrieves the size of the ExoBrowser window.
  gfx::Size PlatformSize();

  // ### PlatformPosition
  //
  // Retrieves the position of the ExoBrowser window.
  gfx::Point PlatformPosition();


#if defined(OS_WIN) && !defined(USE_AURA)
  static ATOM RegisterWindowClass();
  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
  static LRESULT CALLBACK EditWndProc(HWND, UINT, WPARAM, LPARAM);
#elif defined(TOOLKIT_GTK)
  CHROMEGTK_CALLBACK_0(ExoBrowser, gboolean, OnWindowDestroyed);
  CHROMEG_CALLBACK_3(ExoBrowser, gboolean, OnCloseWindowKeyPressed, 
                     GtkAccelGroup*, GObject*, guint, GdkModifierType);
  CHROMEG_CALLBACK_3(ExoBrowser, gboolean, OnNewWindowKeyPressed, 
                     GtkAccelGroup*, GObject*, guint, GdkModifierType);
#endif


  /****************************************************************************/
  /* MEMBERS */
  /****************************************************************************/
  scoped_ptr<ExoBrowserJavaScriptDialogManager> dialog_manager_;

  gfx::NativeWindow                             window_;

  std::map<CONTROL_TYPE, ExoFrame*>             controls_;
  std::map<std::string, ExoFrame*>              pages_;

  ExoBrowserWrap*                               wrapper_;

  bool                                          is_killed_;

#if defined(OS_WIN) && !defined(USE_AURA)
  WNDPROC                                       default_edit_wnd_proc_;
  static HINSTANCE                              instance_handle_;
#elif defined(TOOLKIT_GTK)
  GtkWidget*                                    hbox_;
  GtkWidget*                                    vbox_;

  GtkWidget*                                    control_left_box_;
  GtkWidget*                                    control_right_box_;
  GtkWidget*                                    control_top_box_;
  GtkWidget*                                    control_bottom_box_;

  GtkWidget*                                    pages_box_;
#elif defined(OS_MACOSX)
  NSView*                                       control_left_box_;
  NSLayoutConstraint*                           control_left_constraint_;
  NSView*                                       control_right_box_;
  NSLayoutConstraint*                           control_right_constraint_;
  NSView*                                       control_top_box_;
  NSLayoutConstraint*                           control_top_constraint_;
  NSView*                                       control_bottom_box_;
  NSLayoutConstraint*                           control_bottom_constraint_;
  NSView*                                       horizontal_box_;
  NSView*                                       pages_box_;
#endif
  gfx::NativeView                               visible_page_;

  // A static container of all the open instances. 
  static std::vector<ExoBrowser*>               s_instances;

  friend class ExoBrowserWrap;
  friend class ExoFrameWrap;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowser);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_UI_EXO_BROWSER_H_
