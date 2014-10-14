// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/api/thrust_menu_binding.h"

#include "base/strings/utf_string_conversions.h"

#include "src/api/thrust_session_binding.h"
#include "src/api/thrust_window_binding.h"
#include "src/browser/thrust_menu.h"
#include "src/browser/browser_client.h"
#include "src/api/api.h"

namespace thrust_shell {

ThrustMenuBindingFactory::ThrustMenuBindingFactory()
{
}

ThrustMenuBindingFactory::~ThrustMenuBindingFactory()
{
}

APIBinding* ThrustMenuBindingFactory::Create(
    const unsigned int id,
    scoped_ptr<base::DictionaryValue> args)
{
  return new ThrustMenuBinding(id, args.Pass());
}

ThrustMenuBinding::ThrustMenuBinding(
    const unsigned int id, 
    scoped_ptr<base::DictionaryValue> args)
  : APIBinding("menu", id)
{
  LOG(INFO) << "ThrustMenuBinding Constructor: " << this;
  menu_.reset(new ThrustMenu());
}

ThrustMenuBinding::~ThrustMenuBinding()
{
  LOG(INFO) << "ThrustMenuBinding Destructor: " << this;
  menu_.reset();
}


void
ThrustMenuBinding::CallLocalMethod(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const API::MethodCallback& callback)
{
  std::string err = std::string("");
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "ThrustMenu call [" << method << "]";
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
  else if(method.compare("set_checked") == 0 ||
          method.compare("set_enabled") == 0 ||
          method.compare("set_visible") == 0) {
    int command_id = -1;
    bool value = false;

    args->GetInteger("command_id", &command_id);
    args->GetBoolean("value", &value);

    if(method.compare("set_checked") == 0) {
      menu_->SetChecked(command_id, value);
    }
    if(method.compare("set_enabled") == 0) {
      menu_->SetEnabled(command_id, value);
    }
    if(method.compare("set_visible") == 0) {
      menu_->SetVisible(command_id, value);
    }
  }
  else if(method.compare("set_accelerator") == 0) {
    int command_id = -1;
    std::string accelerator = "";

    args->GetInteger("command_id", &command_id);
    args->GetString("accelerator", &accelerator);

    menu_->SetAccelerator(command_id, accelerator);
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

    ThrustMenu* menu = NULL;

    ThrustMenuBinding* mb = 
      (ThrustMenuBinding*)(API::Get()->GetBinding(menu_id));
    if(mb != NULL) {
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
    int window_id = -1;
    args->GetInteger("window_id", &window_id);

    ThrustWindow* window = NULL;

    ThrustWindowBinding* sb = 
      (ThrustWindowBinding*)(API::Get()->GetBinding(window_id));
    if(sb != NULL) {
      window = sb->GetWindow();
      menu_->AttachToWindow(window);
      LOG(INFO) << "ATTACH TO WINDOW" << window_id;
    }
    else {
      err = "exo_menu_binding:window_not_found";
    }
  }
#if defined(OS_MACOSX)
  else if(method.compare("set_application_menu")) {
    int menu_id = -1;
    args->GetInteger("menu_id", &menu_id);

    ThrustMenu* menu = NULL;

    ThrustMenuBinding* mb = 
      (ThrustMenuBinding*)(API::Get()->GetBinding(menu_id));
    if(mb != NULL) {
      menu = mb->GetMenu();
      ThrustMenu::SetApplicationMenu(menu);
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

ThrustMenu*
ThrustMenuBinding::GetMenu() {
  return menu_.get();
}

} // namespace thrust_shell

