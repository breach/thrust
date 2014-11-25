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

  bool has_frame = true;
  args->GetBoolean("has_frame", &has_frame);

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
        this,
        session,
        GURL(root_url), 
        gfx::Size(width, height), 
        title, 
        icon_path, 
        has_frame));
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

  /* Methods */
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
  else if(method.compare("set_fullscreen") == 0) {
    bool fullscreen;
    args->GetBoolean("fullscreen", &fullscreen);
    window_->SetFullscreen(fullscreen);
  }
  else if(method.compare("set_kiosk") == 0) {
    bool kiosk;
    args->GetBoolean("kiosk", &kiosk);
    window_->SetKiosk(kiosk);
  }
  else if(method.compare("open_devtools") == 0) {
    window_->OpenDevTools();
  }
  else if(method.compare("close_devtools") == 0) {
    window_->CloseDevTools();
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

    window_->Resize(width, height);
  }
  else if(method.compare("close") == 0) {
    window_->Close();
  }
  else if(method.compare("remote") == 0) {
    const base::DictionaryValue* message = NULL;
    args->GetDictionary("message", &message);
    window_->RemoteDispatch(*message);
  }
  /* Accessors */
  else if(method.compare("is_closed") == 0) {
    res->SetBoolean("closed", window_->IsClosed());
  }
  else if(method.compare("size") == 0) {
    res->SetInteger("size.width", window_->GetSize().width());
    res->SetInteger("size.height", window_->GetSize().height());
  }
  else if(method.compare("position") == 0) {
    res->SetInteger("position.x", window_->GetPosition().x());
    res->SetInteger("position.y", window_->GetPosition().y());
  }
  else if(method.compare("is_maximized") == 0) {
    res->SetBoolean("maximized", window_->IsMaximized());
  }
  else if(method.compare("is_minimized") == 0) {
    res->SetBoolean("minimized", window_->IsMinimized());
  }
  else if(method.compare("is_fullscreen") == 0) {
    res->SetBoolean("fullscreen", window_->IsFullscreen());
  }
  else if(method.compare("is_kiosk") == 0) {
    res->SetBoolean("kiosk", window_->IsKiosk());
  }
  else if(method.compare("is_devtools_opened") == 0) {
    res->SetBoolean("opened", window_->IsDevToolsOpened());
  }
  /* Default */
  else {
    err = "thrust_window_binding:method_not_found";
  }

  callback.Run(err, scoped_ptr<base::DictionaryValue>(res).Pass());
}

ThrustWindow*
ThrustWindowBinding::GetWindow() {
  return window_.get();
}

void 
ThrustWindowBinding::EmitClosed()
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  this->EmitEvent("closed", scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void 
ThrustWindowBinding::EmitBlur()
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  this->EmitEvent("blur", scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void 
ThrustWindowBinding::EmitFocus()
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  this->EmitEvent("focus", scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void 
ThrustWindowBinding::EmitUnresponsive()
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  this->EmitEvent("unresponsive", scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void 
ThrustWindowBinding::EmitResponsive()
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  this->EmitEvent("responsive", scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void 
ThrustWindowBinding::EmitWorkerCrashed()
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  this->EmitEvent("worker_crashed", scoped_ptr<base::DictionaryValue>(evt).Pass());
}

void 
ThrustWindowBinding::RemoteSend(
    const base::DictionaryValue& message)
{
  base::DictionaryValue* evt = new base::DictionaryValue;
  evt->Set("message", message.DeepCopy());
  this->EmitEvent("remote", scoped_ptr<base::DictionaryValue>(evt).Pass());
}


} // namespace thrust_shell
