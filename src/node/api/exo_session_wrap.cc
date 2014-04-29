// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "exo_browser/src/node/api/exo_session_wrap.h"

#include "base/time/time.h"

#include "content/public/browser/browser_thread.h"
#include "content/public/browser/web_contents.h"

#include "exo_browser/src/browser/session/exo_session.h"
#include "exo_browser/src/browser/exo_browser.h"
#include "exo_browser/src/browser/exo_frame.h"
#include "exo_browser/src/node/node_thread.h"

using namespace v8;

namespace {

static Local<Object>
ObjectFromCanonicalCookie(
    const net::CanonicalCookie& cc)
{
  Local<Object> cookie_o = Object::New(Isolate::GetCurrent());

  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "source"),
                String::NewFromUtf8(Isolate::GetCurrent(), cc.Source().c_str()));
  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "name"),
                String::NewFromUtf8(Isolate::GetCurrent(), cc.Name().c_str()));
  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "value"),
                String::NewFromUtf8(Isolate::GetCurrent(), cc.Value().c_str()));
  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "domain"),
                String::NewFromUtf8(Isolate::GetCurrent(), cc.Domain().c_str()));
  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "path"),
                String::NewFromUtf8(Isolate::GetCurrent(), cc.Path().c_str()));

  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "creation"),
                Number::New(Isolate::GetCurrent(), 
                            cc.CreationDate().ToInternalValue()));
  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "expiry"),
                Number::New(Isolate::GetCurrent(), 
                            cc.ExpiryDate().ToInternalValue()));
  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "last_access"),
                Number::New(Isolate::GetCurrent(), 
                            cc.LastAccessDate().ToInternalValue()));

  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "secure"),
                v8::Boolean::New(Isolate::GetCurrent(), cc.IsSecure()));
  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "http_only"),
                v8::Boolean::New(Isolate::GetCurrent(), cc.IsHttpOnly()));

  cookie_o->Set(String::NewFromUtf8(Isolate::GetCurrent(), "priority"),
                Number::New(Isolate::GetCurrent(), cc.Priority()));

  return cookie_o;
}

net::CanonicalCookie*
CanonicalCookieFromObject(
    const Local<Object>& cookie_o)
{
  net::CanonicalCookie* cc = net::CanonicalCookie::Create(
      GURL(*String::Utf8Value(
          cookie_o->Get(String::NewFromUtf8(Isolate::GetCurrent(), 
                                            "source"))->ToString())),
      std::string(
        *String::Utf8Value(cookie_o->Get(
            String::NewFromUtf8(Isolate::GetCurrent(), "name"))->ToString())),
      std::string(
        *String::Utf8Value(cookie_o->Get(
            String::NewFromUtf8(Isolate::GetCurrent(), "value"))->ToString())),
      std::string(
        *String::Utf8Value(cookie_o->Get(
            String::NewFromUtf8(Isolate::GetCurrent(), "domain"))->ToString())),
      std::string(
        *String::Utf8Value(cookie_o->Get(
            String::NewFromUtf8(Isolate::GetCurrent(), "path"))->ToString())),
      base::Time::FromInternalValue(cookie_o->Get(
          String::NewFromUtf8(Isolate::GetCurrent(), 
                              "creation"))->ToNumber()->Value()),
      base::Time::FromInternalValue(
        cookie_o->Get(String::NewFromUtf8(Isolate::GetCurrent(), 
                                          "expiry"))->ToNumber()->Value()),
      cookie_o->Get(String::NewFromUtf8(Isolate::GetCurrent(), 
                                        "secure"))->ToBoolean()->Value(),
      cookie_o->Get(String::NewFromUtf8(Isolate::GetCurrent(), 
                                        "http_only"))->ToBoolean()->Value(),
      (net::CookiePriority)cookie_o->Get(
        String::NewFromUtf8(Isolate::GetCurrent(), 
                            "priority"))->ToInteger()->Value());

  if(cc != NULL) {
    cc->SetLastAccessDate(
        base::Time::FromInternalValue(
          cookie_o->Get(
            String::NewFromUtf8(Isolate::GetCurrent(), 
                                "last_access"))->ToNumber()->Value()));
  }

  return cc;
}

}

namespace exo_browser {

Persistent<Function> ExoSessionWrap::s_constructor;

void 
ExoSessionWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(Isolate::GetCurrent(),
                                                      New);
  tpl->SetClassName(String::NewFromUtf8(Isolate::GetCurrent(), "_ExoSession"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  /* Prototype */
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_offTheRecord"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            OffTheRecord)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_addVisitedLink"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            AddVisitedLink)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_clearVisitedLinks"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            ClearVisitedLinks)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_getDevToolsURL"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            GetDevToolsURL)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setCookiesAddCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetCookiesAddCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setCookiesDeleteCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetCookiesDeleteCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), 
                          "_setCookiesUpdateAccessTimeCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetCookiesUpdateAccessTimeCallback)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), 
                          "_setCookiesForceKeepSessionStateCallback"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetCookiesForceKeepSessionStateCallback)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setCookiesLoadForKeyHandler"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetCookiesLoadForKeyHandler)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewFromUtf8(Isolate::GetCurrent(), "_setCookiesFlushHandler"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            SetCookiesFlushHandler)->GetFunction());

  s_constructor.Reset(Isolate::GetCurrent(), tpl->GetFunction());

  exports->Set(String::NewFromUtf8(Isolate::GetCurrent(), "_createExoSession"),
      FunctionTemplate::New(Isolate::GetCurrent(), 
                            CreateExoSession)->GetFunction());
}

ExoSessionWrap::ExoSessionWrap()
: cookies_load_rid_(0)
{
}

ExoSessionWrap::~ExoSessionWrap()
{
  LOG(INFO) << "ExoSessionWrap Destructor";

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::DeleteTask, session_));
}

void 
ExoSessionWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  ExoSessionWrap* session_w = new ExoSessionWrap();
  session_w->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void 
ExoSessionWrap::CreateExoSession(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());
  DCHECK(Isolate::GetCurrent() == args.GetIsolate());

  Local<Function> c = 
    Local<Function>::New(Isolate::GetCurrent(), s_constructor);
  Local<Object> session_o = c->NewInstance();

  /* We keep a Peristent as the object will be returned asynchronously. */
  Persistent<Object> *session_p = new Persistent<Object>();
  session_p->Reset(Isolate::GetCurrent(), session_o);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(session_o);

  /* args[0]: spec = { off_the_record, path } */
  Local<Object> spec = Local<Object>::Cast(args[0]);
  bool off_the_record = 
    spec->Get(String::NewFromUtf8(Isolate::GetCurrent(), 
                                  "off_the_record"))->ToBoolean()->Value();
  std::string path = std::string(
      *String::Utf8Value(Local<String>::Cast(
          spec->Get(String::NewFromUtf8(Isolate::GetCurrent(), "path")))));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::CreateTask, session_w, 
                 off_the_record, path, session_p, cb_p));
}

void
ExoSessionWrap::CreateTask(
    const bool off_the_record,
    const std::string& path,
    Persistent<Object>* session_p,
    Persistent<Function>* cb_p)
{
  session_ = new ExoSession(off_the_record,
                            path,
                            this);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoSessionWrap::CreateCallback, this, session_p, cb_p));
}

void
ExoSessionWrap::CreateCallback(
    Persistent<Object>* session_p,
    Persistent<Function>* cb_p)
{
  HandleScope scope(Isolate::GetCurrent());

  Local<Function> cb = Local<Function>::New(Isolate::GetCurrent(), *cb_p);
  Local<Object> session_o = Local<Object>::New(Isolate::GetCurrent(),
                                               *session_p);

  LOG(INFO) << "CreateCallback ExoSessionWrap";
  Local<v8::Value> argv[1] = { session_o };
  cb->Call(session_o, 1, argv);

  cb_p->Reset();
  delete cb_p;
  session_p->Reset();
  delete session_p;
}

void
ExoSessionWrap::DeleteTask(
    ExoSession* session)
{
  LOG(INFO) << "ExoSessionWrap DeleteTask";
  delete session;
}


/******************************************************************************/
/* WRAPPERS, TASKS */
/******************************************************************************/

void 
ExoSessionWrap::OffTheRecord(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  bool* off_the_record = new bool;

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::OffTheRecordTask, 
                 session_w, off_the_record, cb_p));
}


void
ExoSessionWrap::OffTheRecordTask(
    bool* off_the_record,
    Persistent<Function>* cb_p)
{
  if(session_ != NULL)
    (*off_the_record) = session_->IsOffTheRecord();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoSessionWrap::BooleanCallback, this, cb_p, off_the_record));
}

void 
ExoSessionWrap::AddVisitedLink(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: url */
  std::string url = std::string(
      *String::Utf8Value(Local<String>::Cast(args[0])));

  /* args[1]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[1]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::AddVisitedLinkTask, 
                 session_w, url, cb_p));
}

void
ExoSessionWrap::AddVisitedLinkTask(
    const std::string& url,
    Persistent<Function>* cb_p)
{
  if(session_ != NULL)
    session_->GetVisitedLinkStore()->Add(url);

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoSessionWrap::EmptyCallback, this, cb_p));
}


void 
ExoSessionWrap::ClearVisitedLinks(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());

  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::ClearVisitedLinksTask, 
                 session_w, cb_p));
}

void
ExoSessionWrap::ClearVisitedLinksTask(
    Persistent<Function>* cb_p)
{
  if(session_ != NULL)
    session_->GetVisitedLinkStore()->Clear();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoSessionWrap::EmptyCallback, this, cb_p));
}

void
ExoSessionWrap::GetDevToolsURL(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Persistent<Function> *cb_p = new Persistent<Function>();
  cb_p->Reset(Isolate::GetCurrent(), cb);

  std::string* url = new std::string();

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&ExoSessionWrap::GetDevToolsURLTask, session_w, url, cb_p));
}

void
ExoSessionWrap::GetDevToolsURLTask(
    std::string* url,
    Persistent<Function>* cb_p)
{
  if(session_ != NULL)
    (*url) = session_->GetDevToolsURL().spec();

  NodeThread::Get()->PostTask(
      FROM_HERE,
      base::Bind(&ExoSessionWrap::StringCallback, this, cb_p, url));
}


/******************************************************************************/
/* DISPATCHERS */
/******************************************************************************/
void 
ExoSessionWrap::SetCookiesAddCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  session_w->cookies_add_cb_.Reset(Isolate::GetCurrent(), cb);
}

void 
ExoSessionWrap::DispatchCookiesAdd(
    const net::CanonicalCookie& cc)
{
  HandleScope scope(Isolate::GetCurrent());

  Local<Object> session_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!cookies_add_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), cookies_add_cb_);

    Local<Object> cc_arg = ObjectFromCanonicalCookie(cc);

    Local<v8::Value> argv[1] = { cc_arg };
    cb->Call(session_o, 1, argv);
  }
}

void 
ExoSessionWrap::SetCookiesDeleteCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  session_w->cookies_delete_cb_.Reset(Isolate::GetCurrent(), cb);
}

void 
ExoSessionWrap::DispatchCookiesDelete(
    const net::CanonicalCookie& cc)
{
  HandleScope scope(Isolate::GetCurrent());

  Local<Object> session_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!cookies_delete_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), cookies_delete_cb_);

    Local<Object> cc_arg = ObjectFromCanonicalCookie(cc);

    Local<v8::Value> argv[1] = { cc_arg };
    cb->Call(session_o, 1, argv);
  }
}

void 
ExoSessionWrap::SetCookiesUpdateAccessTimeCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  session_w->cookies_update_access_time_cb_.Reset(Isolate::GetCurrent(), cb);
}

void 
ExoSessionWrap::DispatchCookiesUpdateAccessTime(
    const net::CanonicalCookie& cc)
{
  HandleScope scope(Isolate::GetCurrent());

  Local<Object> session_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!cookies_update_access_time_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), 
                           cookies_update_access_time_cb_);

    Local<Object> cc_arg = ObjectFromCanonicalCookie(cc);

    Local<v8::Value> argv[1] = { cc_arg };
    cb->Call(session_o, 1, argv);
  }
}

void 
ExoSessionWrap::SetCookiesForceKeepSessionStateCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: cb_ */
  Local<Function> cb = Local<Function>::Cast(args[0]);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  session_w->cookies_force_keep_session_state_cb_.Reset(
      Isolate::GetCurrent(), cb);
}

void 
ExoSessionWrap::DispatchCookiesForceKeepSessionState()
{
  HandleScope scope(Isolate::GetCurrent());

  Local<Object> session_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!cookies_force_keep_session_state_cb_.IsEmpty()) {
    Local<Function> cb = 
      Local<Function>::New(Isolate::GetCurrent(), 
                           cookies_force_keep_session_state_cb_);

    cb->Call(session_o, 0, NULL);
  }
}

/******************************************************************************/
/* HANDLERS */
/******************************************************************************/
void
ExoSessionWrap::SetCookiesLoadForKeyHandler(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: hdlr */
  Local<Function> hdlr = Local<Function>::Cast(args[0]);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  session_w->cookies_load_hdlr_.Reset(Isolate::GetCurrent(), hdlr);
}

void
ExoSessionWrap::CookiesLoadForKeyCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  ExoSessionWrap* session_w = 
    ObjectWrap::Unwrap<ExoSessionWrap>(args.This());

  /* args[0]: rid */
  int rid = (Local<Integer>::Cast(args[0]))->Value();
  LoadedCallback cb = session_w->cookies_load_reqs_[rid];
  
  LOG(INFO) << "CookiesLoadForKeyCallback" << " [" << rid << "]";

  /* args[1]: cookies */
  std::vector<net::CanonicalCookie*> ccs;
  Local<Array> cookies = Local<Array>::Cast(args[1]);
  for(unsigned int i = 0; i < cookies->Length(); i ++) {
    net::CanonicalCookie *cc = 
        CanonicalCookieFromObject(
            Local<Object>::Cast(cookies->Get(Integer::New(Isolate::GetCurrent(),
                                                          i))));
    if(cc != NULL)
      ccs.push_back(cc);
  }

  LOG(INFO) << "RETURNED: " << ccs.size();

  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE,
      base::Bind(cb, ccs));
}

void
ExoSessionWrap::CallCookiesLoadForKey(
    const std::string& key,
    const LoadedCallback& cb)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> session_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!cookies_load_hdlr_.IsEmpty()) {
    Local<Function> hdlr = 
      Local<Function>::New(Isolate::GetCurrent(), cookies_load_hdlr_);

    int rid = cookies_load_rid_++;
    cookies_load_reqs_[rid] = cb;
    Local<FunctionTemplate> tpl = 
      FunctionTemplate::New(Isolate::GetCurrent(), CookiesLoadForKeyCallback);

    LOG(INFO) << "CookiesLoadForKey: " << key << " [" << rid << "]";

    Local<String> key_arg = String::NewFromUtf8(Isolate::GetCurrent(),
                                                key.c_str());
    Local<Integer> rid_arg = Integer::New(Isolate::GetCurrent(),
                                          rid);
    Local<Function> cb_arg = tpl->GetFunction();

    Local<v8::Value> argv[3] = { key_arg,
                                 rid_arg,
                                 cb_arg };
    hdlr->Call(session_o, 3, argv);
  }
}

void
ExoSessionWrap::CallCookiesLoad(
    const LoadedCallback& cb)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> session_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!cookies_load_hdlr_.IsEmpty()) {
    Local<Function> hdlr = 
      Local<Function>::New(Isolate::GetCurrent(), cookies_load_hdlr_);

    int rid = cookies_load_rid_++;
    cookies_load_reqs_[rid] = cb;
    Local<FunctionTemplate> tpl = 
      FunctionTemplate::New(Isolate::GetCurrent(), CookiesLoadForKeyCallback);

    LOG(INFO) << "CookiesLoad [" << rid << "]";

    Local<v8::Value> key_arg = Null(Isolate::GetCurrent());
    Local<Integer> rid_arg = Integer::New(Isolate::GetCurrent(), rid);
    Local<Function> cb_arg = tpl->GetFunction();

    Local<v8::Value> argv[3] = { key_arg,
                                 rid_arg,
                                 cb_arg };
    hdlr->Call(session_o, 3, argv);
  }
}

void 
ExoSessionWrap::SetCookiesFlushHandler(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  /* args[0]: hdlr */
  Local<Function> hdlr = Local<Function>::Cast(args[0]);

  ExoSessionWrap* session_w = ObjectWrap::Unwrap<ExoSessionWrap>(args.This());
  session_w->cookies_flush_hdlr_.Reset(Isolate::GetCurrent(), hdlr);
}

void 
ExoSessionWrap::CookiesFlushCallback(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope scope(Isolate::GetCurrent());

  ExoSessionWrap* session_w = 
    ObjectWrap::Unwrap<ExoSessionWrap>(args.This());

  /* args[0]: rid */
  int rid = (Local<Integer>::Cast(args[0]))->Value();
  base::Closure cb = session_w->cookies_flush_reqs_[rid];

  LOG(INFO) << "CookiesFlushCallback" << " [" << rid << "]";

  content::BrowserThread::PostTask(
      content::BrowserThread::IO, FROM_HERE, cb);
}

void 
ExoSessionWrap::CallCookiesFlush(
    const base::Closure& cb)
{
  HandleScope scope(Isolate::GetCurrent());
  Local<Object> session_o = 
    Local<Object>::New(Isolate::GetCurrent(), 
                       this->persistent());

  if(!cookies_flush_hdlr_.IsEmpty()) {
    Local<Function> hdlr = 
      Local<Function>::New(Isolate::GetCurrent(), cookies_flush_hdlr_);

    int rid = cookies_flush_rid_++;
    cookies_flush_reqs_[rid] = cb;
    Local<FunctionTemplate> tpl = FunctionTemplate::New(Isolate::GetCurrent(),
                                                        CookiesFlushCallback);

    LOG(INFO) << "CookiesFlush" << " [" << rid << "]";

    Local<Integer> rid_arg = Integer::New(Isolate::GetCurrent(), rid);
    Local<Function> cb_arg = tpl->GetFunction();

    Local<v8::Value> argv[2] = { rid_arg,
                                 cb_arg };
    hdlr->Call(session_o, 2, argv);
  }
}


} // namespace exo_browser

