// Copyright (c) 2013 Stanislas Polu.
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
  virtual ~ExoBrowserWebContentsViewDelegate();

  // WebContentsViewDelegate Override.
  virtual void ShowContextMenu(
      const content::ContextMenuParams& params) OVERRIDE;
  virtual content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

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
  void ActionPerformed(int id);
#elif defined(OS_WIN)
  virtual void StoreFocus() OVERRIDE;
  virtual void RestoreFocus() OVERRIDE;
  virtual bool Focus() OVERRIDE;
  virtual void TakeFocus(bool reverse) OVERRIDE;
  virtual void SizeChanged(const gfx::Size& size) OVERRIDE;
  void MenuItemSelected(int selection);
#endif

 private:
  content::WebContents* web_contents_;
  content::ContextMenuParams params_;

#if defined(TOOLKIT_GTK)
  ui::OwnedWidgetGtk floating_;
  GtkWidget* expanded_container_;

  CHROMEGTK_CALLBACK_0(ExoBrowserWebContentsViewDelegate, void, 
                       OnContextMenuItemActivated);
#endif

  friend class base::RefCountedThreadSafe<ExoBrowserWebContentsViewDelegate>;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserWebContentsViewDelegate);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_UI_WEB_CONTENTS_VIEW_DELEGATE_H_
