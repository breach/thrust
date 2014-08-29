// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/api/exo_menu_binding.h"

#include "src/browser/exo_menu.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"

namespace exo_shell {

ExoMenuBindingFactory::ExoMenuBindingFactory()
{
}

ExoMenuBindingFactory::~ExoMenuBindingFactory()
{
}

ApiBinding* 
ExoMenuBindingFactory::Create(
    const unsigned int id,
    scoped_ptr<base::DictionaryValue> args)
{
  return new ExoMenuBinding(id, args.Pass());
}


ExoMenuBinding::ExoMenuBinding(
    const unsigned int id, 
    scoped_ptr<base::DictionaryValue> args)
  : ApiBinding("shell", id)
{
  LOG(INFO) << "ExoMenuBinding Constructor";
  ExoMenu *menu = new ExoMenu(this);
  menu_.reset(menu);
}

ExoMenuBinding::~ExoMenuBinding()
{
  LOG(INFO) << "ExoMenuBinding Destructor";
}


void
ExoMenuBinding::LocalCall(
    ApiHandler* handler,
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const ApiHandler::ActionCallback& callback)
{
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "CALL " << method;
  if(method.compare("setApplicationMenu") == 0) {

    // std::string mlbl1 = "MainMenu";

    // std::string lbl1 = "Hello";
    // std::string lbl2 = "World";
    // std::string lbl3 = "Item";

    // base::string16 s16_mlbl1;

    // base::string16 s16_lbl1;
    // base::string16 s16_lbl2;
    // base::string16 s16_lbl3;

    // UTF8ToUTF16(mlbl1.c_str(), mlbl1.length(), &s16_mlbl1);

    // UTF8ToUTF16(lbl1.c_str(), lbl1.length(), &s16_lbl1);
    // UTF8ToUTF16(lbl2.c_str(), lbl2.length(), &s16_lbl2);
    // UTF8ToUTF16(lbl3.c_str(), lbl3.length(), &s16_lbl3);


    // ExoMenu* submenu = new ExoMenu();

    // submenu->InsertItemAt(0,0,s16_lbl1);
    // submenu->InsertItemAt(1,0,s16_lbl2);
    // submenu->InsertSeparatorAt(2);
    // submenu->InsertItemAt(3,0,s16_lbl3);

    // ExoMenu* submenu2 = new ExoMenu();

    // submenu2->InsertItemAt(0,0,s16_lbl1);
    // submenu2->InsertItemAt(1,0,s16_lbl2);
    // submenu2->InsertItemAt(2,0,s16_lbl3);

    // submenu->InsertSubMenuAt(4,0,s16_mlbl1,submenu2);

    // menu_->InsertSubMenuAt(0,0,s16_mlbl1,submenu);

    ExoMenu::SetApplicationMenu(menu_.get());

    // delete submenu;
  }
  else if(method.compare("insertItemAt") == 0) {
    int index, command_id;
    base::string16 s16_label;
    std::string label;
    args->GetInteger("index", &index);
    args->GetInteger("command_id", &command_id);
    args->GetString("label", &label);
    
    UTF8ToUTF16(label.c_str(), label.length(), &s16_label);
    LOG(INFO) << "LABEL::" << label;
    LOG(INFO) << "LABEL`::" << s16_label;
    menu_->InsertItemAt(index, command_id, s16_label);
    LOG(INFO) << "LABEL``::OUT::" << menu_.get()->model_->GetLabelAt(index);
  }
  else if(method.compare("insertSeperatorAt") == 0) {
    int index;
    args->GetInteger("index", &index);
    menu_->InsertSeparatorAt(index);
  } else if (method.compare("insertSubMenuAt") == 0) {
    int index, command_id, submenu_id;
    base::string16 s16_label;
    std::string label;
    args->GetInteger("index", &index);
    args->GetInteger("command_id", &command_id);
    args->GetInteger("submenu_id", &submenu_id);
    args->GetString("label", &label);

    UTF8ToUTF16(label.c_str(), label.length(), &s16_label);

    ApiBinding* binding = handler->GetBindingByTargetID(submenu_id);
    if (binding != NULL) {    
      try {
        ExoMenuBinding* submenu = static_cast<ExoMenuBinding*>(binding);
        menu_->InsertSubMenuAt(index, 
                            command_id,
                            s16_label,
                            submenu->GetMenu());    
      } catch (std::bad_cast &e) {
        LOG(INFO) << "BAD CAST ERROR";
      }
    } else {
      LOG(INFO) << "ERROR Binding is NULL";
    }

  }
  handler = NULL;
  callback.Run(std::string(""), 
               scoped_ptr<base::Value>(res).Pass());
}

ExoMenu*
ExoMenuBinding::GetMenu() {
  return menu_.get();
}

} // namespace exo_shell
