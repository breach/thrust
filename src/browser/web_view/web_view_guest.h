// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_WEB_VIEW_WEB_VIEW_GUEST_H_
#define THRUST_SHELL_BROWSER_WEB_VIEW_WEB_VIEW_GUEST_H_

#include <queue>

#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "content/public/browser/browser_plugin_guest_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/web_contents_delegate.h"
#include "content/public/browser/web_contents_observer.h"

namespace thrust_shell {

class ThrustWindow;

// ## WebViewGuest
//
// Browser-side API implementation for the <webview> tag. It's in charge of
// maintaining the association between the guest WebContents and the embedder
// WebContents. Receives events issued from the guest and realys them to the
// embedder.
//
// A WebViewGuest is also a WebContentsObserver on the guest WebContents of the
// associated <webview> tag. WebViewGuest is created on attachment. That is, when a 
// guest WebContents is associated with a particular embedder WebContents. 
// This happens on either initial navigation or through the use of the New 
// Window API, when a new window is attached to a particular <webview>.
class WebViewGuest : public content::BrowserPluginGuestDelegate,
                     public content::NotificationObserver,
                     public content::WebContentsDelegate,
                     public content::WebContentsObserver {
public:

  /****************************************************************************/
  /* STATIC API */
  /****************************************************************************/
  static WebViewGuest* Create(int guest_instance_id);

  static WebViewGuest* FromWebContents(content::WebContents* web_contents);

  // Returns guestview::kInstanceIDNone if |contents| does not correspond to a
  // WebViewGuest.
  static int GetViewInstanceId(content::WebContents* contents);
  // Parses partition related parameters from |extra_params|.
  // |storage_partition_id| is the parsed partition ID and |persist_storage|
  // specifies whether or not the partition is in memory.
  static void ParsePartitionParam(const base::DictionaryValue* extra_params,
                                  std::string* storage_partition_id,
                                  bool* persist_storage);

  /****************************************************************************/
  /* BROWSER_PLUGIN_GUEST_DELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual void Destroy() OVERRIDE FINAL;
  virtual void DidAttach() OVERRIDE FINAL;
  virtual void ElementSizeChanged(const gfx::Size& old_size,
                                  const gfx::Size& new_size) OVERRIDE FINAL;
  virtual int GetGuestInstanceID() const OVERRIDE;
  virtual void GuestSizeChanged(const gfx::Size& old_size,
                                const gfx::Size& new_size) OVERRIDE FINAL;
  virtual void RegisterDestructionCallback(
      const DestructionCallback& callback) OVERRIDE FINAL;
  virtual void WillAttach(
      content::WebContents* embedder_web_contents,
      const base::DictionaryValue& extra_params) OVERRIDE FINAL;

  virtual content::WebContents* CreateNewGuestWindow(
      const content::WebContents::CreateParams& create_params) OVERRIDE;
  /*
  virtual void RequestPointerLockPermission(
      bool user_gesture,
      bool last_unlocked_by_target,
      const base::Callback<void(bool)>& callback) OVERRIDE;
  */

  /****************************************************************************/
  /* NOTIFICATION_OBSERVER IMPLEMENTATION */
  /****************************************************************************/
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  /****************************************************************************/
  /* WEBVIEW API */
  /****************************************************************************/
  // ### LoadUrl
  //
  // Loads the specified url
  void LoadUrl(const GURL& url);

  // ### SetZoom
  //
  // Set the zoom factor.
  void SetZoom(double zoom_factor);

  // ### GetZoom
  //
  // Returns the current zoom factor.
  double GetZoom();

  // ### Go
  //
  // If possible, navigate the guest to |relative_index| entries away from the
  // current navigation entry.
  // ```
  // @relative_index {int}
  // ```
  void Go(int relative_index);

  // ### Reload
  //
  // Reload the guest.
  // ```
  // @ignore_cache {bool}
  // ```
  void Reload(bool ignore_cache);

  // ### Stop
  //
  // Stop loading the guest.
  void Stop();


  /****************************************************************************/
  /* PUBLIC API */
  /****************************************************************************/
  void Init(content::WebContents* guest_web_contents);

  // Toggles autosize mode for this GuestView.
  void SetAutoSize(bool enabled,
                   const gfx::Size& min_size,
                   const gfx::Size& max_size);

  content::WebContents* embedder_web_contents() const {
    return embedder_web_contents_;
  }

  // Returns the guest WebContents.
  content::WebContents* guest_web_contents() const {
    return guest_web_contents_;
  }

  // Returns whether this guest has an associated embedder.
  bool attached() const { return !!embedder_web_contents_; }

  // Returns the instance ID of the <*view> element.
  int view_instance_id() const { return view_instance_id_; }

  // Returns the instance ID of the guest WebContents.
  int guest_instance_id() const { return guest_instance_id_; }

  // Returns the user browser context of the embedder.
  content::BrowserContext* browser_context() const { return browser_context_; }

  // Returns the embedder's process ID.
  int embedder_render_process_id() const { return embedder_render_process_id_; }

  /****************************************************************************/
  /* PROTECTED & PRIVATE API */
  /****************************************************************************/
 protected:
  WebViewGuest(int guest_instance_id);
  virtual ~WebViewGuest();

 private:
  class EmbedderWebContentsObserver;

  ThrustWindow* GetThrustWindow();

  /****************************************************************************/
  /* WEBCONTENTSOBSERVER IMPLEMENTATION */
  /****************************************************************************/
  virtual void DidStopLoading(
      content::RenderViewHost* render_view_host) OVERRIDE FINAL;
  virtual void RenderViewReady() OVERRIDE FINAL;
  virtual void WebContentsDestroyed() OVERRIDE FINAL;
  /*
  virtual void DidCommitProvisionalLoadForFrame(
      int64 frame_id,
      const base::string16& frame_unique_name,
      bool is_main_frame,
      const GURL& url,
      content::PageTransition transition_type,
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void DidFailProvisionalLoad(
      int64 frame_id,
      const base::string16& frame_unique_name,
      bool is_main_frame,
      const GURL& validated_url,
      int error_code,
      const base::string16& error_description,
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void DidStartProvisionalLoadForFrame(
      int64 frame_id,
      int64 parent_frame_id,
      bool is_main_frame,
      const GURL& validated_url,
      bool is_error_page,
      bool is_iframe_srcdoc,
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void DocumentLoadedInFrame(
      int64 frame_id,
      content::RenderViewHost* render_view_host) OVERRIDE;
  virtual void UserAgentOverrideSet(const std::string& user_agent) OVERRIDE;
  */

  /****************************************************************************/
  /* WEBCONTENTSDELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual bool ShouldCreateWebContents(
      content::WebContents* web_contents,
      int route_id,
      WindowContainerType window_container_type,
      const base::string16& frame_name,
      const GURL& target_url,
      const std::string& partition_id,
      content::SessionStorageNamespace* session_storage_namespace) OVERRIDE;
  virtual void CloseContents(content::WebContents* source) OVERRIDE;
  virtual content::WebContents* OpenURLFromTab(
      content::WebContents* source,
      const content::OpenURLParams& params) OVERRIDE;
  virtual bool AddMessageToConsole(content::WebContents* source,
                                   int32 level,
                                   const base::string16& message,
                                   int32 line_no,
                                   const base::string16& source_id) OVERRIDE;
  virtual void HandleKeyboardEvent(
      content::WebContents* source,
      const content::NativeWebKeyboardEvent& event) OVERRIDE;

  /****************************************************************************/
  /* DATA FIELDS */
  /****************************************************************************/
  content::WebContents*                     guest_web_contents_;
  content::WebContents*                     embedder_web_contents_;
  int                                       embedder_render_process_id_;
  content::BrowserContext*                  browser_context_;
  // |guest_instance_id_| is a profile-wide unique identifier for a guest
  // WebContents.
  const int                                 guest_instance_id_;
  // |view_instance_id_| is an identifier that's unique within a particular
  // embedder RenderViewHost for a particular <*view> instance.
  int                                       view_instance_id_;
  bool                                      initialized_;
  content::NotificationRegistrar            notification_registrar_;
  // Stores the current zoom factor.
  double                                    current_zoom_factor_;
  DestructionCallback                       destruction_callback_;
  // The extra parameters associated with this GuestView passed
  // in from JavaScript. This will typically be the view instance ID,
  // the API to use, and view-specific parameters. These parameters
  // are passed along to new guests that are created from this guest.
  scoped_ptr<base::DictionaryValue>         extra_params_;
  scoped_ptr<EmbedderWebContentsObserver>   embedder_web_contents_observer_;
  // The size of the container element.
  gfx::Size                                 element_size_;
  // The size of the guest content. Note: In autosize mode, the container
  // element may not match the size of the guest.
  gfx::Size                                 guest_size_;
  // Indicates whether autosize mode is enabled or not.
  bool                                      auto_size_enabled_;
  // The maximum size constraints of the container element in autosize mode.
  gfx::Size                                 max_auto_size_;
  // The minimum size constraints of the container element in autosize mode.
  gfx::Size                                 min_auto_size_;
  // This is used to ensure pending tasks will not fire after this object is
  // destroyed.
  base::WeakPtrFactory<WebViewGuest>        weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebViewGuest);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_WEB_VIEW_WEB_VIEW_GUEST_H_
