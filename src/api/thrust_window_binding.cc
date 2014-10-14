// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/api/thrust_window_binding.h"

#include "src/api/thrust_session_binding.h"
#include "src/browser/thrust_window.h"
#include "src/browser/browser_client.h"
#include "src/api/api.h"

namespace thrust_shell {

ThrustWindowBindingFactory::ThrustWindowBindingFactory()
{
}

ThrustWindowBindingFactory::~ThrustWindowBindingFactory()
{
}

APIBinding* ThrustWindowBindingFactory::Create(
    const unsigned int id,
    scoped_ptr<base::DictionaryValue> args)
{
  return new ThrustWindowBinding(id, args.Pass());
}


ThrustWindowBinding::ThrustWindowBinding(
    const unsigned int id, 
    scoped_ptr<base::DictionaryValue> args)
  : APIBinding("window", id)
{
  LOG(INFO) << "ThrustWindowBinding Constructor [" << this << "] " << id_;

  std::string root_url = "http://google.com";
  args->GetString("root_url", &root_url);

  int width = 800;
  int height = 600;

  args->GetInteger("size.width", &width);
  args->GetInteger("size.height", &height);

  std::string title = "ThrustShell";
  args->GetString("title", &title);

  std::string icon_path = "";
  args->GetString("icon_path", &icon_path);

  ThrustSession* session = NULL;

  int session_id = -1;
  args->GetInteger("session_id", &session_id);

  ThrustSessionBinding* sb = 
    (ThrustSessionBinding*)(API::Get()->GetBinding(session_id));
  if(sb != NULL) {
    session = sb->GetSession();
  }
  else {
    session = ThrustShellBrowserClient::Get()->system_session();
  }

  window_.reset(ThrustWindow::CreateNew(
        session,
        GURL(root_url), 
        gfx::Size(width, height), 
        title, 
        icon_path, 
        true));
}

ThrustWindowBinding::~ThrustWindowBinding()
{
  LOG(INFO) << "ThrustWindowBinding Destructor [" << this << "] " << id_;
  window_.reset();
}


void
ThrustWindowBinding::CallLocalMethod(
    const std::string& method,
    scoped_ptr<base::DictionaryValue> args,
    const API::MethodCallback& callback)
{
  std::string err = std::string("");
  base::DictionaryValue* res = new base::DictionaryValue;

  LOG(INFO) << "ThrustWindow call [" << method << "]";
  if(method.compare("show") == 0) {
    window_->Show();
  }
  else if(method.compare("focus") == 0) {
    bool focus = true;
    args->GetBoolean("focus", &focus);
    window_->Focus(focus);
  }
  else if(method.compare("maximize") == 0) {
    window_->Maximize();
  }
  else if(method.compare("unmaximize") == 0) {
    window_->UnMaximize();
  }
  else if(method.compare("minimize") == 0) {
    window_->Minimize();
  }
  else if(method.compare("restore") == 0) {
    window_->Restore();
  }
  else if(method.compare("set_title") == 0) {
    std::string title = "";
    args->GetString("title", &title);
    window_->SetTitle(title);
  }
  else if(method.compare("move") == 0) {
	int x, y;
	args->GetInteger("x", &x);
	args->GetInteger("y", &y);

	window_->Move(x, y);
  }
  else if(method.compare("resize") == 0) {
	int width, height;
	args->GetInteger("width", &width);
	args->GetInteger("height", &height);

	LOG(INFO) << "calling window_->Resize(" << width << ", " << height << ")";
	window_->Resize(width, height);
  }
  else if(method.compare("close") == 0) {
    window_->Close();
  }
  else if(method.compare("is_closed") == 0) {
    res->SetBoolean("is_closed", window_->is_closed());
  }
  else if(method.compare("size") == 0) {
    res->SetInteger("size.width", window_->size().width());
    res->SetInteger("size.height", window_->size().height());
  }
  else if(method.compare("position") == 0) {
    res->SetInteger("position.x", window_->position().x());
    res->SetInteger("position.y", window_->position().y());
  }
  else {
    err = "thrust_window_binding:method_not_found";
  }

  callback.Run(err, scoped_ptr<base::Value>(res).Pass());
}

ThrustWindow*
ThrustWindowBinding::GetWindow() {
  return window_.get();
}

} // namespace thrust_shell
