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
  ExoMenu *menu = new ExoMenu();
  menu_.reset(menu);
}

ExoMenuBinding::~ExoMenuBinding()
{
  LOG(INFO) << "ExoMenuBinding Destructor";
}


void
ExoMenuBinding::LocalCall(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const ApiHandler::ActionCallback& callback)
{
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "CALL " << method;
  if(method.compare("setApplicationMenu") == 0) {
    std::string lbl1 = "Hello";
    std::string lbl2 = "World";
    std::string lbl3 = "Item";

    base::string16 s16_lbl1;
    base::string16 s16_lbl2;
    base::string16 s16_lbl3;

    UTF8ToUTF16(lbl1.c_str(), lbl1.length(), &s16_lbl1);
    UTF8ToUTF16(lbl2.c_str(), lbl2.length(), &s16_lbl2);
    UTF8ToUTF16(lbl3.c_str(), lbl3.length(), &s16_lbl3);


    menu_->InsertItemAt(0,0,s16_lbl1);
    menu_->InsertItemAt(1,0,s16_lbl2);
    menu_->InsertItemAt(2,0,s16_lbl3);

    ExoMenu::SetApplicationMenu(menu_.get());
  }
  else if(method.compare("insertItemAt") == 0) {
    int index, id;
    base::string16 s16_label;
    std::string label;
    args->GetInteger("index", &index);
    args->GetInteger("id", &id);
    args->GetString("label", &label);
    UTF8ToUTF16(label.c_str(), label.length(), &s16_label);

    menu_->InsertItemAt(index, id, s16_label);
  }
  else if(method.compare("insertSeperatorAt") == 0) {
    int index;
    args->GetInteger("index", &index);
    menu_->InsertSeparatorAt(index);
  }
  callback.Run(std::string(""), 
               scoped_ptr<base::Value>(res).Pass());
}


} // namespace exo_shell
