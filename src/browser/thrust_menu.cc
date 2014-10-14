// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2013 GitHub, Inc.
// See the LICENSE file.

#include "src/browser/thrust_menu.h"

#include "src/browser/thrust_window.h"
#include "src/browser/ui/accelerator_util.h"

namespace thrust_shell {

ThrustMenu::ThrustMenu()
  : model_(new ui::SimpleMenuModel(this)),
    parent_(NULL) 
{
}

ThrustMenu::~ThrustMenu() {
}

bool 
ThrustMenu::IsCommandIdChecked(
    int command_id) const 
{
  if(checked_.find(command_id) != checked_.end()) {
    return checked_.find(command_id)->second;
  }
  return false;
}

bool 
ThrustMenu::IsCommandIdEnabled(
    int command_id) const 
{
  if(enabled_.find(command_id) != enabled_.end()) {
    return enabled_.find(command_id)->second;
  }
  return true;
}

bool 
ThrustMenu::IsCommandIdVisible(
    int command_id) const 
{
  if(visible_.find(command_id) != visible_.end()) {
    return visible_.find(command_id)->second;
  }
  return true;
}

bool 
ThrustMenu::GetAcceleratorForCommandId(
    int command_id,
    ui::Accelerator* accelerator) 
{
  if(accelerator_.find(command_id) != accelerator_.end()) {
    return accelerator_util::StringToAccelerator(accelerator_[command_id], accelerator);
  }
  return false;
}

void 
ThrustMenu::ExecuteCommand(
    int command_id, 
    int event_flags) 
{
  LOG(INFO) << "---+++++++++++++_-_____________________ EXECUTE " << command_id << " " << event_flags;
  /* TODO(spolu); */
}

void 
ThrustMenu::MenuWillShow(
    ui::SimpleMenuModel* source) 
{
}

void 
ThrustMenu::MenuClosed(
    ui::SimpleMenuModel* source) 
{
}

void 
ThrustMenu::AttachToWindow(ThrustWindow* window) 
{
  window->SetMenu(model_.get());
}

void 
ThrustMenu::InsertItemAt(
    int index, 
    int command_id, 
    const base::string16& label) 
{
  model_->InsertItemAt(index, command_id, label);
}

void 
ThrustMenu::InsertSeparatorAt(
    int index) 
{
  model_->InsertSeparatorAt(index, ui::NORMAL_SEPARATOR);
}

void 
ThrustMenu::InsertCheckItemAt(
    int index,
    int command_id,
    const base::string16& label) 
{
  model_->InsertCheckItemAt(index, command_id, label);
}

void 
ThrustMenu::InsertRadioItemAt(
    int index,
    int command_id,
    const base::string16& label,
    int group_id) 
{
  model_->InsertRadioItemAt(index, command_id, label, group_id);
}

void 
ThrustMenu::InsertSubMenuAt(
    int index,
    int command_id,
    const base::string16& label,
    ThrustMenu* menu) 
{
  menu->parent_ = this;
  model_->InsertSubMenuAt(index, command_id, label, menu->model_.get());
}

void 
ThrustMenu::SetSublabel(
    int index, 
    const base::string16& sublabel) 
{
  model_->SetSublabel(index, sublabel);
}

void 
ThrustMenu::Clear() 
{
  model_->Clear();
}

void
ThrustMenu::SetChecked(
    int command_id,
    bool checked)
{
  checked_[command_id] = checked;
}

void
ThrustMenu::SetEnabled(
    int command_id,
    bool enabled)
{
  enabled_[command_id] = enabled;
}

void
ThrustMenu::SetVisible(
    int command_id,
    bool visible)
{
  visible_[command_id] = visible;
}

void
ThrustMenu::SetAccelerator(
    int command_id,
    std::string accelerator)
{
  accelerator_[command_id] = accelerator;
}

int 
ThrustMenu::GetIndexOfCommandId(
    int command_id) 
{
  return model_->GetIndexOfCommandId(command_id);
}

int 
ThrustMenu::GetItemCount() const 
{
  return model_->GetItemCount();
}

int 
ThrustMenu::GetCommandIdAt(
    int index) const 
{
  return model_->GetCommandIdAt(index);
}

base::string16 
ThrustMenu::GetLabelAt(
    int index) const 
{
  return model_->GetLabelAt(index);
}

base::string16 
ThrustMenu::GetSublabelAt(
    int index) const 
{
  return model_->GetSublabelAt(index);
}

bool 
ThrustMenu::IsItemCheckedAt(
    int index) const 
{
  return model_->IsItemCheckedAt(index);
}

bool 
ThrustMenu::IsEnabledAt(
    int index) const 
{
  return model_->IsEnabledAt(index);
}

bool 
ThrustMenu::IsVisibleAt(
    int index) const 
{
  return model_->IsVisibleAt(index);
}

} // namespace thrust_shell
