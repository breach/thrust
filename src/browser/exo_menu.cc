// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2013 GitHub, Inc.
// See the LICENSE file.

#include "src/browser/exo_menu.h"

#include "src/browser/exo_shell.h"
#include "src/browser/ui/accelerator_util.h"

namespace exo_shell {

ExoMenu::ExoMenu()
  : model_(new ui::SimpleMenuModel(this)),
    parent_(NULL) 
{
}

ExoMenu::~ExoMenu() {
}

bool 
ExoMenu::IsCommandIdChecked(
    int command_id) const 
{
  if(checked_.find(command_id) != checked_.end()) {
    return checked_.find(command_id)->second;
  }
  return false;
}

bool 
ExoMenu::IsCommandIdEnabled(
    int command_id) const 
{
  if(enabled_.find(command_id) != enabled_.end()) {
    return enabled_.find(command_id)->second;
  }
  return true;
}

bool 
ExoMenu::IsCommandIdVisible(
    int command_id) const 
{
  if(visible_.find(command_id) != visible_.end()) {
    return visible_.find(command_id)->second;
  }
  return true;
}

bool 
ExoMenu::GetAcceleratorForCommandId(
    int command_id,
    ui::Accelerator* accelerator) 
{
  if(accelerator_.find(command_id) != accelerator_.end()) {
    return accelerator_util::StringToAccelerator(accelerator_[command_id], accelerator);
  }
  return false;
}

void 
ExoMenu::ExecuteCommand(
    int command_id, 
    int event_flags) 
{
  LOG(INFO) << "---+++++++++++++_-_____________________ EXECUTE " << command_id << " " << event_flags;
  /* TODO(spolu); */
}

void 
ExoMenu::MenuWillShow(
    ui::SimpleMenuModel* source) 
{
}

void 
ExoMenu::MenuClosed(
    ui::SimpleMenuModel* source) 
{
}

void 
ExoMenu::AttachToShell(ExoShell* shell) 
{
  shell->SetMenu(model_.get());
}

void 
ExoMenu::InsertItemAt(
    int index, 
    int command_id, 
    const base::string16& label) 
{
  model_->InsertItemAt(index, command_id, label);
}

void 
ExoMenu::InsertSeparatorAt(
    int index) 
{
  model_->InsertSeparatorAt(index, ui::NORMAL_SEPARATOR);
}

void 
ExoMenu::InsertCheckItemAt(
    int index,
    int command_id,
    const base::string16& label) 
{
  model_->InsertCheckItemAt(index, command_id, label);
}

void 
ExoMenu::InsertRadioItemAt(
    int index,
    int command_id,
    const base::string16& label,
    int group_id) 
{
  model_->InsertRadioItemAt(index, command_id, label, group_id);
}

void 
ExoMenu::InsertSubMenuAt(
    int index,
    int command_id,
    const base::string16& label,
    ExoMenu* menu) 
{
  menu->parent_ = this;
  model_->InsertSubMenuAt(index, command_id, label, menu->model_.get());
}

void 
ExoMenu::SetSublabel(
    int index, 
    const base::string16& sublabel) 
{
  model_->SetSublabel(index, sublabel);
}

void 
ExoMenu::Clear() 
{
  model_->Clear();
}

void
ExoMenu::SetChecked(
    int command_id,
    bool checked)
{
  checked_[command_id] = checked;
}

void
ExoMenu::SetEnabled(
    int command_id,
    bool enabled)
{
  enabled_[command_id] = enabled;
}

void
ExoMenu::SetVisible(
    int command_id,
    bool visible)
{
  visible_[command_id] = visible;
}

void
ExoMenu::SetAccelerator(
    int command_id,
    std::string accelerator)
{
  accelerator_[command_id] = accelerator;
}

int 
ExoMenu::GetIndexOfCommandId(
    int command_id) 
{
  return model_->GetIndexOfCommandId(command_id);
}

int 
ExoMenu::GetItemCount() const 
{
  return model_->GetItemCount();
}

int 
ExoMenu::GetCommandIdAt(
    int index) const 
{
  return model_->GetCommandIdAt(index);
}

base::string16 
ExoMenu::GetLabelAt(
    int index) const 
{
  return model_->GetLabelAt(index);
}

base::string16 
ExoMenu::GetSublabelAt(
    int index) const 
{
  return model_->GetSublabelAt(index);
}

bool 
ExoMenu::IsItemCheckedAt(
    int index) const 
{
  return model_->IsItemCheckedAt(index);
}

bool 
ExoMenu::IsEnabledAt(
    int index) const 
{
  return model_->IsEnabledAt(index);
}

bool 
ExoMenu::IsVisibleAt(
    int index) const 
{
  return model_->IsVisibleAt(index);
}

} // namespace exo_shell
