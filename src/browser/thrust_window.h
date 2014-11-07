// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_THRUST_WINDOW_H_
#define THRUST_SHELL_BROWSER_THRUST_WINDOW_H_

#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "ui/gfx/image/image.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "vendor/brightray/browser/default_web_contents_delegate.h"
#include "vendor/brightray/browser/inspectable_web_contents_delegate.h"
#include "vendor/brightray/browser/inspectable_web_contents_impl.h"

#if defined(USE_AURA)
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"
#endif

#if defined(OS_MACOSX)
struct CGContext;
#endif

class GURL;

namespace base {
class CommandLine;
}

namespace content {
class WebContents;
struct NativeWebKeyboardEvent;
struct FileChooserParams;
}

namespace ui {
class MenuModel;
}

namespace thrust_shell {

class ThrustSession;
class ThrustWindowBinding;
class ThrustShellJavaScriptDialogManager;
class ThrustShellWebDialogHelper;

class GlobalMenuBarX11;

// ### ThrustWindow
//
// This represents an ThrustShell window. The window opens on a `root_url` 
// provided at creation. The window exposes only one webcontents with support 
// for the <webview> tag.
//
// The ThrustWindow lives on the BrowserThread::UI thread
class ThrustWindow : public brightray::DefaultWebContentsDelegate,
                     public brightray::InspectableWebContentsDelegate,
                     public content::WebContentsObserver,
#if defined(USE_AURA)
                     public views::WidgetDelegateView,
                     public views::WidgetObserver,
#elif defined(OS_MACOSX)
#endif
                     public content::NotificationObserver {
public:
  /****************************************************************************/
  /* STATIC INTERFACE */
  /****************************************************************************/
  // ### CreateNew
  //
  // Creates a new ThrustWindow with the specified `root_url`
  // ```
  // @binding   {ThrustWindowBinding} the binding associated
  // @session   {ThrustSession} the session to use
  // @root_url  {GURL} the main document root url
  // @size      {Size} the initial size of the window
  // @title     {string} the title to use
  // @icon_path {string} icon_path (no effect on OSX)
  // @has_frame {boolean} has a frame
  // ```
  static ThrustWindow* CreateNew(
      ThrustWindowBinding* binding,
      ThrustSession* session,
      const GURL& root_url,
      const gfx::Size& size,
      const std::string& title,
      const std::string& icon_path,
      const bool has_frame);

  // ### CreateNew
  //
  // Creates a new ThrustWindow out of an existing WebContents
  // ```
  // @binding      {ThrustWindowBinding} the binding associated
  // @web_contents {WebContents} the web_contents to use
  // @size         {Size} the initial size of the window
  // @title        {string} the title to use
  // @icon_path    {string} icon_path (no effect on OSX)
  // @has_frame    {boolean} has a frame
  // ```
  static ThrustWindow* CreateNew(
      ThrustWindowBinding* binding,
      content::WebContents* web_contents,
      const gfx::Size& size,
      const std::string& title,
      const std::string& icon_path,
      const bool has_frame);

  // ### instances
  //
  // Getter for all the currently working ThrustWindow instances.
  static std::vector<ThrustWindow*>& instances() { return s_instances; }

  // ### CloseAll
  //
  // Closes all open ThrustWindows
  static void CloseAll();

  /****************************************************************************/
  /* PUBLIC INTERFACE */
  /****************************************************************************/
  // ### ~ThrustWindow
  ~ThrustWindow();

  // ### Show
  //
  // Initially show the window
  void Show() { PlatformShow(); }

  // ### Focus
  //
  // Focuses the window
  void Focus(bool focus) { PlatformFocus(focus); }

  // ### Maximize
  //
  // Maximize the window
  void Maximize() { PlatformMaximize(); }

  // ### UnMaximize
  //
  // UnMaximize the window
  void UnMaximize() { PlatformUnMaximize(); }

  // ### Minimize
  //
  // Minimize the window
  void Minimize() { PlatformMinimize(); }

  // ### Restore
  //
  // Restore the window
  void Restore() { PlatformRestore(); }

  // ### SetTitle
  //
  // Sets the window title
  void SetTitle(const std::string& title);

  // ### SetFullscreen
  //
  // Sets the window in kiosk mode
  void SetFullscreen(bool fullscreen) { PlatformSetFullscreen(fullscreen); }

  // ### IsFullscreen
  //
  // Returns whether the window is in kiosk mode
  bool IsFullscreen() { return PlatformIsFullscreen(); }

  // ### SetKiosk
  //
  // Sets the window in kiosk mode
  void SetKiosk(bool kiosk) { PlatformSetKiosk(kiosk); }

  // ### IsKiosk
  //
  // Returns whether the window is in kiosk mode
  bool IsKiosk() { return PlatformIsKiosk(); }

  // ### Close
  //
  // Closes the window and reclaim underlying WebContents
  void Close();

  // ### OpenDevTools
  //
  // Opens the DevTools view for the main document frame
  void OpenDevTools();

  // ### CloseDevTools
  //
  // Closes the DevTools view for the main document frame
  void CloseDevTools();

  // ### IsDevToolsOpened
  //
  // Returns wether the DevTools View is opened
  bool IsDevToolsOpened();

  // ### Move
  //
  // Moves the window
  void Move(int x, int y);

  // ### Resize
  //
  // Resizes the window
  void Resize(int width, int height);

  // ### IsClosed
  //
  // Returns whether the window is closed or not
  bool IsClosed() { return is_closed_; }

  // ### HasFrame
  //
  // Returns wether the window has frame or not
  bool HasFrame() { return has_frame_; }

  // ### GetSize
  //
  // Retrieves the native Window size
  gfx::Size GetSize() { return PlatformSize(); }

  // ### GetPosition
  //
  // Retrieves the native Window position
  gfx::Point GetPosition() { return PlatformPosition(); }

  // ### IsMaximized
  //
  // Retrieves whether the window is maximized
  bool IsMaximized() { return PlatformIsMaximized(); }

  // ### IsMinimized
  //
  // Retrieves whether the window is minimized
  bool IsMinimized() { return PlatformIsMinimized(); }

  // ### GetNativeWindow
  //
  // Returns the NativeWindow for this Shell
  gfx::NativeWindow GetNativeWindow() { return PlatformGetNativeWindow(); }

  // ### GetWebContents
  //
  // Returns the underlying web_contents
  content::WebContents* GetWebContents() const;

  // ### GetBinding
  //
  // Returns the binding for that window
  ThrustWindowBinding* GetBinding() const { return binding_; }

  // ### GetDraggableRegion
  //
  // Returns the draggable region
  SkRegion* GetDraggableRegion() const { 
    return draggable_region_.get(); 
  }

  /****************************************************************************/
  /* WEBCONTENTSDELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;

  virtual void RequestToLockMouse(content::WebContents* web_contents,
                                  bool user_gesture,
                                  bool last_unlocked_by_target) OVERRIDE;
  virtual bool CanOverscrollContent() const OVERRIDE;

  virtual void CloseContents(content::WebContents* source) OVERRIDE;

  virtual content::JavaScriptDialogManager*
    GetJavaScriptDialogManager() OVERRIDE;

  virtual void ActivateContents(content::WebContents* contents) OVERRIDE;
  virtual void DeactivateContents(content::WebContents* contents) OVERRIDE;

  virtual void RendererUnresponsive(content::WebContents* source) OVERRIDE;
  virtual void RendererResponsive(content::WebContents* source) OVERRIDE;
  virtual void WorkerCrashed(content::WebContents* source) OVERRIDE;

  virtual content::ColorChooser* OpenColorChooser(
      content::WebContents* web_contents,
      SkColor color,
      const std::vector<content::ColorSuggestion>& suggestions) OVERRIDE;
  virtual void RunFileChooser(
      content::WebContents* web_contents,
      const content::FileChooserParams& params) OVERRIDE;
  virtual void EnumerateDirectory(content::WebContents* web_contents,
                                  int request_id,
                                  const base::FilePath& path) OVERRIDE;

  /****************************************************************************/
  /* NOTIFICATIONOBSERFVER IMPLEMENTATION */
  /****************************************************************************/
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  /****************************************************************************/
  /* WEBCONTENTSOBSERVER IMPLEMENTATION */
  /****************************************************************************/
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE; 
  virtual bool OnMessageReceived(const IPC::Message& message,
                                 content::RenderFrameHost* render_frame_host) OVERRIDE; 

  /****************************************************************************/
  /* WEBVIEWGUEST MESSAGE HANDLING */
  /****************************************************************************/
  void CreateWebViewGuest(const base::DictionaryValue& params,
                          int* guest_instance_id); 
  void DestroyWebViewGuest(int guest_instance_id); 

  void WebViewEmit(int guest_instance_id,
                   const std::string type,
                   const base::DictionaryValue& params);

  void WebViewGuestSetAutoSize(int guest_instance_id,
                               const base::DictionaryValue& params);
  void WebViewGuestLoadUrl(int guest_instance_id,
                           const std::string& url);
  void WebViewGuestGo(int guest_instance_id,
                      int relative_index);
  void WebViewGuestReload(int guest_instance_id,
                          bool ignore_cache);
  void WebViewGuestStop(int guest_instance_id);
  void WebViewGuestSetZoom(int guest_instance_id,
                           double zoom_factor);
  void WebViewGuestFind(int guest_instance_id,
                        int request_id,
                        const std::string& search_text,
                        const base::DictionaryValue& options);
  void WebViewGuestStopFinding(int guest_instance_id,
                               const std::string& action);
  void WebViewGuestInsertCSS(int guest_instance_id,
                             const std::string& css);
  void WebViewGuestExecuteScript(int guest_instance_id,
                                 const std::string& script);
  void WebViewGuestOpenDevTools(int guest_instance_id);
  void WebViewGuestCloseDevTools(int guest_instance_id);
  void WebViewGuestIsDevToolsOpened(int guest_instance_id,
                                    bool* open); 
  void WebViewGuestJavaScriptDialogClosed(int guest_instance_id,
                                          bool success, 
                                          const std::string& response); 

#if defined(OS_MACOSX)
  /****************************************************************************/
  /* OSX SPECIFIC INTERFACE */
  /****************************************************************************/
  void ClipWebView();
#elif defined(USE_AURA)
  /****************************************************************************/
  /* AURA SPECIFIC INTERFACE */
  /****************************************************************************/
  void AttachMenu(ui::MenuModel* menu);
  void DetachMenu();
#endif


protected:
  // ### CloseImmediately
  //
  // Closes the window immediately (WebContents already gone)
  void CloseImmediately();

  // ### inspectable_web_contents
  //
  // Returns the underlying inspectable_web_contents
  brightray::InspectableWebContentsImpl* inspectable_web_contents() const {
    return static_cast<brightray::InspectableWebContentsImpl*>(
        inspectable_web_contents_.get());
  }

private:
  /****************************************************************************/
  /* PRIVATE INTERFACE */
  /****************************************************************************/
  explicit ThrustWindow(
      ThrustWindowBinding* binding,
      content::WebContents* web_contents,
      const gfx::Size& size,
      const std::string& title,
      const std::string& icon_path,
      const bool has_frame);

  void DestroyWebContents();

#if defined(USE_AURA)
  /****************************************************************************/
  /* VIEWS::WIDGETOBSERVER IMPLEMENTATION */
  /****************************************************************************/
  virtual void OnWidgetActivationChanged(
      views::Widget* widget, bool active) OVERRIDE;

  /****************************************************************************/
  /* VIEWS::WIDGETDELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual void DeleteDelegate() OVERRIDE;
  virtual views::View* GetInitiallyFocusedView() OVERRIDE;
  virtual bool CanResize() const OVERRIDE;
  virtual bool CanMaximize() const OVERRIDE;
  virtual base::string16 GetWindowTitle() const OVERRIDE;
  virtual bool ShouldHandleSystemCommands() const OVERRIDE;
  virtual gfx::ImageSkia GetWindowAppIcon() OVERRIDE;
  virtual gfx::ImageSkia GetWindowIcon() OVERRIDE;
  virtual views::Widget* GetWidget() OVERRIDE;
  virtual const views::Widget* GetWidget() const OVERRIDE;
  virtual views::View* GetContentsView() OVERRIDE;
  virtual bool ShouldDescendIntoChildForEventHandling(
     gfx::NativeView child,
     const gfx::Point& location) OVERRIDE;
  virtual views::ClientView* CreateClientView(views::Widget* widget) OVERRIDE;
  virtual views::NonClientFrameView* CreateNonClientFrameView(
      views::Widget* widget) OVERRIDE;

#elif defined(OS_MACOSX)
  /****************************************************************************/
  /* OSX SPECIFIC HELPER METHODS */
  /****************************************************************************/
  void InstallView();
  void UninstallView();
#endif

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
  // Creates the window GUI.
  void PlatformCreateWindow(const gfx::Size& size);

  // ### PlatformShow
  //
  // Initially Show the window
  void PlatformShow();

  // ### PlatformFocus
  //
  // Focuses the window
  void PlatformFocus(bool focus);

  // ### PlatformMaximize
  //
  // Maximizes the window
  void PlatformMaximize();

  // ### PlatformUnMaximize
  //
  // Maximizes the window
  void PlatformUnMaximize();

  // ### PlatformMinimize
  //
  // Minimize the window
  void PlatformMinimize();

  // ### PlatformRestore
  //
  // Restore the window
  void PlatformRestore();

  // ### PlatformSetTitle
  //
  // Set the title of window.
  void PlatformSetTitle(const std::string& title);

  // ### PlatformSetFullscreen
  //
  // Sets the window in kiosk mode
  void PlatformSetFullscreen(bool fullscreen);

  // ### IsFullscreen
  //
  // Returns whether the window is in kiosk mode
  bool PlatformIsFullscreen();

  // ### PlatformSetKiosk
  //
  // Sets the window in kiosk mode
  void PlatformSetKiosk(bool kiosk);

  // ### IsKiosk
  //
  // Returns whether the window is in kiosk mode
  bool PlatformIsKiosk();

  // ### PlatformClose
  //
  // Let each platform close the window.
  void PlatformClose();

  // ### PlatformCloseImmediately
  //
  // Let each platform close the window.
  void PlatformCloseImmediately();


  //
  // ### PlatformMove
  //
  // Moves the position of the window.
  void PlatformMove(int x, int y);

  // ### PlatformResize
  //
  // Resize the window.
  void PlatformResize(int width, int height);

  // ### PlatformSetContentSize
  //
  // Sets the size of the shell content
  void PlatformSetContentSize(int width, int height);

  // ### PlatformContentSize
  //
  // Retrieves the size of the ThrustWindow content
  gfx::Size PlatformContentSize();

  // ### PlatformSize
  //
  // Retrieves the size of the window.
  gfx::Size PlatformSize();

  // ### PlatformPosition
  //
  // Retrieves the position of the window.
  gfx::Point PlatformPosition();

  // ### PlatformIsMaximized
  //
  // Retrieves whether the window is maximized
  bool PlatformIsMaximized();

  // ### IsMinimized
  //
  // Retrieves whether the window is minimized
  bool PlatformIsMinimized();

  // ### PlatformGetNativeWindow
  //
  // Returns the NativeWindow for this Shell
  gfx::NativeWindow PlatformGetNativeWindow();

#if defined(USE_AURA)
  gfx::Rect ContentBoundsToWindowBounds(const gfx::Rect& bounds);
#endif

  /****************************************************************************/
  /* MEMBERS */
  /****************************************************************************/
  ThrustWindowBinding*                             binding_;
  scoped_ptr<ThrustShellWebDialogHelper>           web_dialog_helper_;
  scoped_ptr<ThrustShellJavaScriptDialogManager>   dialog_manager_;
  content::NotificationRegistrar                   registrar_;

#if defined(USE_AURA)
  scoped_ptr<views::Widget>                        window_;
#if defined(USE_X11)
  scoped_ptr<GlobalMenuBarX11>                     global_menu_bar_;
#endif
#elif defined(OS_MACOSX)
  gfx::NativeWindow                                window_;
  bool                                             is_kiosk_;
#endif
  bool                                             is_closed_;
  gfx::ImageSkia                                   icon_;
  std::string                                      title_;
  bool                                             has_frame_;
  scoped_ptr<SkRegion>                             draggable_region_;

  scoped_ptr<brightray::InspectableWebContents>    inspectable_web_contents_;

  // A static container of all the open instances.
  static std::vector<ThrustWindow*>                s_instances;

  friend class ThrustMenu;

  DISALLOW_COPY_AND_ASSIGN(ThrustWindow);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_THRUST_WINDOW_H_
