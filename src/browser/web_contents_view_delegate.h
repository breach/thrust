// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UI_WEB_CONTENTS_VIEW_DELEGATE_H_
#define EXO_BROWSER_BROWSER_UI_WEB_CONTENTS_VIEW_DELEGATE_H_

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/context_menu_params.h"

#if defined(TOOLKIT_GTK)
#include "ui/base/gtk/gtk_signal.h"
#include "ui/base/gtk/owned_widget_gtk.h"
#endif

namespace exo_browser {

class ExoBrowserWebContentsViewDelegate : 
    public content::WebContentsViewDelegate,
    public base::RefCountedThreadSafe<ExoBrowserWebContentsViewDelegate> {
 public:
  explicit ExoBrowserWebContentsViewDelegate(
      content::WebContents* web_contents);

  // WebContentsViewDelegate Override.
  virtual void ShowContextMenu(
      content::RenderFrameHost* render_frame_host,
      const content::ContextMenuParams& params) OVERRIDE;
  virtual content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

  /****************************************************************************/
  /* EXECUTOR CLASS */
  /****************************************************************************/
  class Executor : public base::RefCountedThreadSafe<Executor> {
   public:
     // ### Executor
     // 
     // This class serves as a RefCountedThreadSafe object to be executed to
     // display the context menu once the items have been retrieved
     Executor(content::WebContents* web_contents)
      : web_contents_(web_contents) {}
    // ### BuildContextMenu
    //
    // Constructs a context menu based on the string received and call the
    // `trigger` callback if a menu item is clicked
    // ```
    // @menu    {vector<string>} the list of menu items (empty string ~ seperator)
    // @trigger {Callback<int>} the callback to call when a menu is fired
    // ```
    void BuildContextMenu(
        const std::vector<std::string>& items,
        const base::Callback<void(const int)>& trigger);

#if defined(TOOLKIT_GTK)
    CHROMEGTK_CALLBACK_0(Executor, void, 
                         OnContextMenuItemActivated);
#elif defined(OS_MACOSX)
    void ActionPerformed(int index);
#endif

   private:
    ~Executor() {}

    content::WebContents* web_contents_;

    friend class base::RefCountedThreadSafe<
       ExoBrowserWebContentsViewDelegate::Executor>;
  };

#if defined(TOOLKIT_GTK)
  virtual void Initialize(GtkWidget* expanded_container,
                          ui::FocusStoreGtk* focus_store) OVERRIDE;
  virtual gfx::NativeView GetNativeView() const OVERRIDE;
  virtual void Focus() OVERRIDE;
  virtual gboolean OnNativeViewFocusEvent(GtkWidget* widget,
                                          GtkDirectionType type,
                                          gboolean* return_value) OVERRIDE;
#elif defined(OS_MACOSX)
  virtual NSObject<RenderWidgetHostViewMacDelegate>*
      CreateRenderWidgetHostViewDelegate(
          content::RenderWidgetHost* render_widget_host) OVERRIDE;
#endif

 private:
  virtual ~ExoBrowserWebContentsViewDelegate();

  content::WebContents* web_contents_;
  content::ContextMenuParams params_;

#if defined(TOOLKIT_GTK)
  ui::OwnedWidgetGtk floating_;
  GtkWidget* expanded_container_;
#endif

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserWebContentsViewDelegate);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_UI_WEB_CONTENTS_VIEW_DELEGATE_H_
