// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/ui/web_contents_view_delegate.h"

#include "base/command_line.h"
#include "ui/base/gtk/focus_store_gtk.h"
#include "ui/base/gtk/gtk_floating_container.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/context_menu_params.h"
#include "content/shell/common/shell_switches.h"
#include "exo_browser/src/node/node_thread.h"
#include "exo_browser/src/browser/ui/exo_frame.h"
#include "exo_browser/src/node/api/exo_frame_wrap.h"

using namespace content;

using WebKit::WebContextMenuData;

namespace exo_browser {

WebContentsViewDelegate* 
CreateExoBrowserWebContentsViewDelegate(
    WebContents* web_contents) {
  return new ExoBrowserWebContentsViewDelegate(web_contents);
}

ExoBrowserWebContentsViewDelegate::ExoBrowserWebContentsViewDelegate(
    WebContents* web_contents)
    : web_contents_(web_contents),
      floating_(gtk_floating_container_new()) 
{
}

ExoBrowserWebContentsViewDelegate::~ExoBrowserWebContentsViewDelegate() 
{
  LOG(INFO) << "WebContentsViewDelegate Destructor";
  floating_.Destroy();
}

void 
ExoBrowserWebContentsViewDelegate::ShowContextMenu(
    const ContextMenuParams& params) 
{
  base::Callback<void(const std::vector<std::string>&, 
                      const base::Callback<void(const int)>&)> callback = 
    base::Bind(&ExoBrowserWebContentsViewDelegate::BuildContextMenu, this);

  ExoFrame* exo_frame = ExoFrame::ExoFrameForWebContents(web_contents_);
  if(exo_frame && exo_frame->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::CallBuildContextMenu, 
                   exo_frame->wrapper_, params, callback));
  }
}

void
ExoBrowserWebContentsViewDelegate::BuildContextMenu(
    const std::vector<std::string>& items,
    const base::Callback<void(const int)>& trigger)
{
  if(items.size() > 0) {
    GtkWidget* menu = gtk_menu_new();

    for(unsigned int i = 0; i < items.size(); i ++) {
      std::string item = items[i];
      std::stringstream ss;
      ss << i;
      std::string index = ss.str();

      if(item.length() == 0) {
        GtkWidget* navigate_separator = gtk_separator_menu_item_new();
        gtk_menu_append(GTK_MENU(menu), navigate_separator);
      }
      else {
        GtkWidget* menu_item = gtk_menu_item_new_with_label(item.c_str());
        gtk_menu_append(GTK_MENU(menu), menu_item);
        gtk_widget_set_name(menu_item, index.c_str());
        g_signal_connect(
            menu_item,
            "activate",
            G_CALLBACK(OnContextMenuItemActivatedThunk),
            this);
        /*
        gtk_widget_set_sensitive(back_menu,
            web_contents_->GetController().CanGoBack());
        */
      }
    }
    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, GDK_CURRENT_TIME);
  }

  /*
  params_ = params;
  bool has_link = !params_.unfiltered_link_url.is_empty();
  bool has_selection = !params_.selection_text.empty();

  if (params_.media_type == WebContextMenuData::MediaTypeNone &&
      !has_link &&
      !has_selection &&
      !params_.is_editable) {
  }
  if (params_.is_editable) {
  } 
  else if (has_selection) {
  }
  */
}

WebDragDestDelegate* 
ExoBrowserWebContentsViewDelegate::GetDragDestDelegate() 
{
  return NULL;
}

void 
ExoBrowserWebContentsViewDelegate::Initialize(
    GtkWidget* expanded_container,
    ui::FocusStoreGtk* focus_store) 
{
  expanded_container_ = expanded_container;

  gtk_container_add(GTK_CONTAINER(floating_.get()), expanded_container_);
  gtk_widget_show(floating_.get());
}

gfx::NativeView 
ExoBrowserWebContentsViewDelegate::GetNativeView() const 
{
  return floating_.get();
}

void 
ExoBrowserWebContentsViewDelegate::Focus() 
{
  GtkWidget* widget = web_contents_->GetView()->GetContentNativeView();
  if(widget)
    gtk_widget_grab_focus(widget);
}

gboolean 
ExoBrowserWebContentsViewDelegate::OnNativeViewFocusEvent(
    GtkWidget* widget,
    GtkDirectionType type,
    gboolean* return_value) {
  return false;
}

void 
ExoBrowserWebContentsViewDelegate::OnContextMenuItemActivated(
    GtkWidget* widget)
{
  int idx = atoi(gtk_widget_get_name(widget));

  ExoFrame* exo_frame = ExoFrame::ExoFrameForWebContents(web_contents_);
  if(exo_frame && exo_frame->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::CallTriggerContextMenuItem, 
                   exo_frame->wrapper_, idx));
  }
}

} // namespace exo_browser
