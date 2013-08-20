// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.
//
#ifndef BREACH_BROWSER_UI_BREACH_WEB_CONTENTS_VIEW_DELEGATE_H_
#define BREACH_BROWSER_UI_BREACH_WEB_CONTENTS_VIEW_DELEGATE_H_

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view_delegate.h"
#include "content/public/common/context_menu_params.h"

#if defined(TOOLKIT_GTK)
#include "ui/base/gtk/gtk_signal.h"
#include "ui/base/gtk/owned_widget_gtk.h"
#endif

namespace breach {

class BreachWebContentsViewDelegate : public content::WebContentsViewDelegate {
 public:
  explicit BreachWebContentsViewDelegate(content::WebContents* web_contents);
  virtual ~BreachWebContentsViewDelegate();

  // WebContentsViewDelegate Override.
  virtual void ShowContextMenu(
      const content::ContextMenuParams& params) OVERRIDE;
  virtual content::WebDragDestDelegate* GetDragDestDelegate() OVERRIDE;

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
          RenderWidgetHost* render_widget_host) OVERRIDE;
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

  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnBackMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnForwardMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnReloadMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnOpenURLMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnCutMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnCopyMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnPasteMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnDeleteMenuActivated);
  CHROMEGTK_CALLBACK_0(BreachWebContentsViewDelegate, void,
                       OnInspectMenuActivated);
#endif

  DISALLOW_COPY_AND_ASSIGN(BreachWebContentsViewDelegate);
};

}  // namespace breach

#endif // BREACH_BROWSER_UI_BREACH_WEB_CONTENTS_VIEW_DELEGATE_H_
