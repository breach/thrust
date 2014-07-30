// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#include "src/api/exo_browser_binding.h"

#include "src/browser/exo_browser.h"

namespace exo_browser {

ExoBrowserBindingFactory::ExoBrowserBindingFactory()
{
}

ExoBrowserBindingFactory::~ExoBrowserBindingFactory()
{
}

ApiBinding* ExoBrowserBindingFactory::Create(
    const unsigned int id,
    scoped_ptr<base::DictionaryValue> args)
{
  return new ExoBrowserBinding(id, args.Pass());
}



ExoBrowserBinding::ExoBrowserBinding(
    const unsigned int id, 
    scoped_ptr<base::DictionaryValue> args)
  : ApiBinding("exo_browser", id)
{

  std::string root_url = "http://google.com";
  args->GetString("root_url", &root_url);

  int width = 800;
  int height = 600;

  args->GetInteger("size.width", &width);
  args->GetInteger("size.height", &height);

  std::string title = "ExoBrowser";
  args->GetString("title", &title);

  std::string icon_path = "";
  args->GetString("icon_path", &icon_path);

  browser_.reset(ExoBrowser::CreateNew(
        GURL(root_url), 
        gfx::Size(width, height), 
        title, 
        icon_path, 
        true));
}

ExoBrowserBinding::~ExoBrowserBinding()
{
  LOG(INFO) << "ExoBrowserBinding Destructor";
  browser_.reset();
}


void
ExoBrowserBinding::LocalCall(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const ApiHandler::ActionCallback& callback)
{
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "CALL " << method;
  if(method.compare("show") == 0) {
    browser_->Show();
  }
  if(method.compare("focus") == 0) {
    bool focus = true;
    args->GetBoolean("focus", &focus);
    browser_->Focus(focus);
  }
  else if(method.compare("maximize") == 0) {
    browser_->Maximize();
  }
  else if(method.compare("unmaximize") == 0) {
    browser_->UnMaximize();
  }
  else if(method.compare("minimize") == 0) {
    browser_->Minimize();
  }
  else if(method.compare("restore") == 0) {
    browser_->Restore();
  }
  else if(method.compare("set_title") == 0) {
    std::string title = "";
    args->GetString("title", &title);
    browser_->SetTitle(title);
  }
  else if(method.compare("close") == 0) {
    browser_->Close();
  }
  else if(method.compare("is_closed") == 0) {
    res->SetBoolean("is_closed", browser_->is_closed());
  }
  else if(method.compare("size") == 0) {
    res->SetInteger("size.width", browser_->size().width());
    res->SetInteger("size.height", browser_->size().height());
  }
  else if(method.compare("position") == 0) {
    res->SetInteger("position.x", browser_->position().x());
    res->SetInteger("position.y", browser_->position().y());
  }

  callback.Run(std::string(""), 
               scoped_ptr<base::Value>(res).Pass());
}


} // namespace exo_browser
