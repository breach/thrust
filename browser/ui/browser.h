// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.
#ifndef BREACH_BROWSER_UI_BROWSER_H_
#define BREACH_BROWSER_UI_BROWSER_H_


#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/web_contents_delegate.h"
#include "ipc/ipc_channel.h"
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

class BreachDevToolsFrontend;
class BreachJavaScriptDialogManager;

// This represents one window of the Breach browser, i.e. all the UI including
// controls and the web content area
class Browser : public content::WebContentsDelegate,
                public content::NotificationObserver {
 public:
  static const int kDefaultWindowWidthDip;
  static const int kDefaultWindowHeightDip;

  virtual ~Browser();

  void LoadURL(const GURL& url);
  void LoadURLForFrame(const GURL& url, const std::string& frame_name);
  void GoBackOrForward(int offset);
  void Reload();
  void Stop();
  void UpdateNavigationControls();
  void Close();
  void ShowDevTools();
  void CloseDevTools();
  gfx::Size Size();
  gfx::Point Position();
#if (defined(OS_WIN) && !defined(USE_AURA)) || defined(TOOLKIT_GTK)
  // Resizes the main window to the given dimensions.
  void SizeTo(int width, int height);
#endif

  // Do one time initialization at application startup.
  static void Initialize();

  static Browser* CreateNewWindow(content::BrowserContext* browser_context,
                                  const GURL& url,
                                  content::SiteInstance* site_instance,
                                  int routing_id,
                                  const gfx::Size& initial_size);

  // Returns the Browser object corresponding to the given RenderViewHost.
  static Browser* FromRenderViewHost(content::RenderViewHost* rvh);

  // Returns the currently open windows.
  static std::vector<Browser*>& windows() { return windows_; }

  // Closes all windows and returns. This runs a message loop.
  static void CloseAllWindows();

  // Closes all windows and exits.
  static void PlatformExit();

  // Specify if that Browser was closed (and should not be used anymore)
  bool IsClosed() { return is_closed_; }

  content::WebContents* web_contents() const { return web_contents_.get(); }
  gfx::NativeWindow window() { return window_; }

#if defined(OS_MACOSX)
  // Public to be called by an ObjC bridge object.
  void ActionPerformed(int control);
  void URLEntered(std::string url_string);
#endif

  // WebContentsDelegate
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;
  virtual void LoadingStateChanged(content::WebContents* source) OVERRIDE;
  virtual void ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                          bool enter_fullscreen) OVERRIDE;
  virtual bool IsFullscreenForTabOrPending(
      const content::WebContents* web_contents) const OVERRIDE;
  virtual void RequestToLockMouse(content::WebContents* web_contents,
                                  bool user_gesture,
                                  bool last_unlocked_by_target) OVERRIDE;
  virtual void CloseContents(content::WebContents* source) OVERRIDE;
  virtual bool CanOverscrollContent() const OVERRIDE;
  virtual void WebContentsCreated(content::WebContents* source_contents,
                                  int64 source_frame_id,
                                  const string16& frame_name,
                                  const GURL& target_url,
                                  content::WebContents* new_contents) OVERRIDE;
  virtual void DidNavigateMainFramePostCommit(
      content::WebContents* web_contents) OVERRIDE;
  virtual content::JavaScriptDialogManager* 
    GetJavaScriptDialogManager() OVERRIDE;
#if defined(OS_MACOSX)
  virtual void HandleKeyboardEvent(
      content::WebContents* source,
      const NativeWebKeyboardEvent& event) OVERRIDE;
#endif
  virtual bool AddMessageToConsole(content::WebContents* source,
                                   int32 level,
                                   const string16& message,
                                   int32 line_no,
                                   const string16& source_id) OVERRIDE;
  virtual void RendererUnresponsive(content::WebContents* source) OVERRIDE;
  virtual void ActivateContents(content::WebContents* contents) OVERRIDE;
  virtual void DeactivateContents(content::WebContents* contents) OVERRIDE;
  virtual void WorkerCrashed(content::WebContents* source) OVERRIDE;

 private:
  enum UIControl {
    BACK_BUTTON,
    FORWARD_BUTTON,
    STOP_BUTTON
  };

  class DevToolsWebContentsObserver;

  explicit Browser(content::WebContents* web_contents);

  // Helper to create a new Browser given a newly created WebContents.
  static Browser* CreateBrowser(content::WebContents* web_contents,
                                const gfx::Size& initial_size);

  // Helper for one time initialization of application
  static void PlatformInitialize(const gfx::Size& default_window_size);

  // All the methods that begin with Platform need to be implemented by the
  // platform specific Browser implementation.
  // Called from the destructor to let each platform do any necessary cleanup.
  void PlatformCleanUp();
  // Creates the main window GUI.
  void PlatformCreateWindow(int width, int height);
  // Links the WebContents into the newly created window.
  void PlatformSetContents();
  // Resize the content area and GUI.
  void PlatformResizeSubViews();
  // Enable/disable a button.
  void PlatformEnableUIControl(UIControl control, bool is_enabled);
  // Updates the url in the url bar.
  void PlatformSetAddressBarURL(const GURL& url);
  // Sets whether the spinner is spinning.
  void PlatformSetIsLoading(bool loading);
  // Set the title of shell window
  void PlatformSetTitle(const string16& title);

  gfx::NativeView GetContentView();

  // NotificationObserver
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  void OnDevToolsWebContentsDestroyed();

#if defined(OS_WIN) && !defined(USE_AURA)
  static ATOM RegisterWindowClass();
  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
  static LRESULT CALLBACK EditWndProc(HWND, UINT, WPARAM, LPARAM);
#elif defined(TOOLKIT_GTK)
  CHROMEGTK_CALLBACK_0(Browser, void, OnBackButtonClicked);
  CHROMEGTK_CALLBACK_0(Browser, void, OnForwardButtonClicked);
  CHROMEGTK_CALLBACK_0(Browser, void, OnReloadButtonClicked);
  CHROMEGTK_CALLBACK_0(Browser, void, OnStopButtonClicked);
  CHROMEGTK_CALLBACK_0(Browser, void, OnURLEntryActivate);
  CHROMEGTK_CALLBACK_0(Browser, gboolean, OnWindowDestroyed);

  CHROMEG_CALLBACK_3(Browser, gboolean, OnCloseWindowKeyPressed, GtkAccelGroup*,
                     GObject*, guint, GdkModifierType);
  CHROMEG_CALLBACK_3(Browser, gboolean, OnNewWindowKeyPressed, GtkAccelGroup*,
                     GObject*, guint, GdkModifierType);
  CHROMEG_CALLBACK_3(Browser, gboolean, OnHighlightURLView, GtkAccelGroup*,
                     GObject*, guint, GdkModifierType);
  CHROMEG_CALLBACK_3(Browser, gboolean, OnReloadKeyPressed, GtkAccelGroup*,
                     GObject*, guint, GdkModifierType);
#endif

  scoped_ptr<BreachJavaScriptDialogManager> dialog_manager_;

  scoped_ptr<content::WebContents> web_contents_;

  scoped_ptr<DevToolsWebContentsObserver> devtools_observer_;
  BreachDevToolsFrontend* devtools_frontend_;

  bool is_fullscreen_;

  gfx::NativeWindow window_;
  gfx::NativeEditView url_edit_view_;

  // Notification manager
  content::NotificationRegistrar registrar_;

#if defined(OS_WIN) && !defined(USE_AURA)
  WNDPROC default_edit_wnd_proc_;
  static HINSTANCE instance_handle_;
#elif defined(TOOLKIT_GTK)
  GtkWidget* vbox_;

  GtkToolItem* back_button_;
  GtkToolItem* forward_button_;
  GtkToolItem* reload_button_;
  GtkToolItem* stop_button_;

  GtkWidget* spinner_;
  GtkToolItem* spinner_item_;

  int content_width_;
  int content_height_;
  int ui_elements_height_; // height of menubar, toolbar, etc.
#endif

  bool is_closed_;

  // A container of all the open windows. We use a vector so we can keep track
  // of ordering.
  static std::vector<Browser*> windows_;

  // True if the destructur of Browser should post a quit closure on the current
  // message loop if the destructed Browser object was the last one.
  static bool quit_message_loop_;
};

} // namespace breach

#endif // BREACH_BROWSER_UI_BROWSER_H_
