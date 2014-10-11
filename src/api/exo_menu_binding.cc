// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/api/exo_menu_binding.h"

#include "base/strings/utf_string_conversions.h"

#include "src/api/exo_session_binding.h"
#include "src/api/exo_shell_binding.h"
#include "src/browser/exo_menu.h"
#include "src/browser/browser_client.h"
#include "src/api/api.h"

namespace exo_shell {

ExoMenuBindingFactory::ExoMenuBindingFactory()
{
}

ExoMenuBindingFactory::~ExoMenuBindingFactory()
{
}

APIBinding* ExoMenuBindingFactory::Create(
    const unsigned int id,
    scoped_ptr<base::DictionaryValue> args)
{
  return new ExoMenuBinding(id, args.Pass());
}

ExoMenuBinding::ExoMenuBinding(
    const unsigned int id, 
    scoped_ptr<base::DictionaryValue> args)
  : APIBinding("menu", id)
{
  LOG(INFO) << "ExoMenuBinding Constructor: " << this;
  menu_.reset(new ExoMenu());
}

ExoMenuBinding::~ExoMenuBinding()
{
  LOG(INFO) << "ExoMenuBinding Destructor: " << this;
  menu_.reset();
}


void
ExoMenuBinding::CallLocalMethod(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const API::MethodCallback& callback)
{
  std::string err = std::string("");
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "ExoMenu call [" << method << "]";
  if(method.compare("insert_item_at") == 0 ||
     method.compare("insert_check_item_at") == 0 ||
     method.compare("insert_radio_item_at") == 0 ||
     method.compare("insert_separator_at") == 0 ||
    method.compare("set_sub_label") == 0) {
    int index = 0;
    int command_id = -1;
    int group_id = -1;
    std::string label = "NO-LABEL";

    args->GetString("label", &label);
    args->GetInteger("index", &index);
    args->GetInteger("command_id", &command_id);
    args->GetInteger("group_id", &group_id);

    if(method.compare("insert_item_at") == 0) {
      menu_->InsertItemAt(index, command_id, base::UTF8ToUTF16(label));
    }
    if(method.compare("insert_check_item_at") == 0) {
      menu_->InsertCheckItemAt(index, command_id, base::UTF8ToUTF16(label));
    }
    if(method.compare("insert_radio_item_at") == 0) {
      menu_->InsertRadioItemAt(index, command_id, base::UTF8ToUTF16(label), group_id);
    }
    if(method.compare("insert_separator_at") == 0) {
      menu_->InsertSeparatorAt(index);
    }
    if(method.compare("set_sub_label") == 0) {
      menu_->SetSublabel(index, base::UTF8ToUTF16(label));
    }
  }
  else if(method.compare("insert_submenu_at") == 0) {
    int index = 0;
    int command_id = -1;
    std::string label = "NO-LABEL";
    int menu_id = -1;

    args->GetInteger("menu_id", &menu_id);
    args->GetString("label", &label);
    args->GetInteger("index", &index);
    args->GetInteger("command_id", &command_id);

    ExoMenu* menu = NULL;

    ExoMenuBinding* mb = 
      (ExoMenuBinding*)(API::Get()->GetBinding(menu_id));
    if(mb != NULL) {
      LOG(INFO) << " INSERT SUBMENU AT: " << menu_id;
      menu = mb->GetMenu();
      menu_->InsertSubMenuAt(index, command_id, base::UTF8ToUTF16(label), menu);
    }
    else {
      err = "exo_menu_binding:menu_not_found";
    }
  }
  else if(method.compare("clear") == 0) {
    menu_->Clear();
  }
  else if(method.compare("attach") == 0) {
    int shell_id = -1;
    args->GetInteger("shell_id", &shell_id);

    ExoShell* shell = NULL;

    ExoShellBinding* sb = 
      (ExoShellBinding*)(API::Get()->GetBinding(shell_id));
    if(sb != NULL) {
      shell = sb->GetShell();
      menu_->AttachToShell(shell);
      LOG(INFO) << "ATTACH TO SHELL" << shell_id;
    }
    else {
      err = "exo_menu_binding:shell_not_found";
    }
  }
#if defined(OS_MACOSX)
  else if(method.compare("set_application_menu")) {
    int menu_id = -1;
    args->GetInteger("menu_id", &menu_id);

    ExoMenu* menu = NULL;

    ExoMenuBinding* mb = 
      (ExoMenuBinding*)(API::Get()->GetBinding(menu_id));
    if(mb != NULL) {
      menu = mb->GetMenu();
      ExoMenu::SetApplicationMenu(menu);
    }
    else {
      err = "exo_menu_binding:menu_not_found";
    }
  }
#endif
  else {
    err = "exo_menu_binding:method_not_found";
  }

  /*
  else if(method.compare("is_closed") == 0) {
    res->SetBoolean("is_closed", shell_->is_closed());
  }
  else if(method.compare("size") == 0) {
    res->SetInteger("size.width", shell_->size().width());
    res->SetInteger("size.height", shell_->size().height());
  }
  else if(method.compare("position") == 0) {
    res->SetInteger("position.x", shell_->position().x());
    res->SetInteger("position.y", shell_->position().y());
  }
  */

  callback.Run(err, scoped_ptr<base::Value>(res).Pass());
}

ExoMenu*
ExoMenuBinding::GetMenu() {
  return menu_.get();
}

} // namespace exo_shell

