// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/exo_menu.h"

#include "base/auto_reset.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/string16.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "ui/gfx/codec/png_codec.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/common/renderer_preferences.h"
#include "content/public/browser/favicon_status.h"

#include "src/common/switches.h"
#include "src/browser/browser_main_parts.h"
#include "src/browser/browser_client.h"
#include "src/browser/dialog/javascript_dialog_manager.h"
#include "src/browser/dialog/file_select_helper.h"
#include "src/common/messages.h"

using namespace content;

namespace exo_shell {

ExoMenu::ExoMenu()
    : model_(new ui::SimpleMenuModel(this)) {
}
ExoMenu::~ExoMenu() {

}

void 
ExoMenu::InsertItemAt(
    int index, int command_id, const base::string16& label) {
  model_->InsertItemAt(index, command_id, label);
}

void 
ExoMenu::InsertSeparatorAt(int index) {
  model_->InsertSeparatorAt(index, ui::NORMAL_SEPARATOR);
}

bool 
ExoMenu::IsCommandIdChecked(int command_id) const {
  return false;
}

bool 
ExoMenu::IsCommandIdEnabled(int command_id) const {
  return false;
}

bool 
ExoMenu::IsCommandIdVisible(int command_id) const {
  return false;
}

bool 
ExoMenu::GetAcceleratorForCommandId(
        int command_id
    ,   ui::Accelerator* accelerator) {
  return false;
}

bool 
ExoMenu::IsItemForCommandIdDynamic(int command_id) const {
  return false;
}

base::string16 
ExoMenu::GetLabelForCommandId(int command_id) const {
  base::string16 str16;
  return str16;
}

base::string16 
ExoMenu::GetSublabelForCommandId(int command_id) const {
  base::string16 str16;
  return str16;
}

void 
ExoMenu::ExecuteCommand(int command_id, int event_flags) {

}

void 
ExoMenu::MenuWillShow(ui::SimpleMenuModel* source) {

}

void
ExoMenu::InsertSubMenuAt(int index,
                           int command_id,
                           const base::string16& label,
                           ExoMenu* menu) {
  menu->parent_ = this;
  model_->InsertSubMenuAt(index, command_id, label, menu->model_.get());
}
// void AttachToWindow(Window* window) const{

// }

void
ExoMenu::Popup() {

}


} // namespace exo_menu


