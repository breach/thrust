// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#define BUILDING_NODE_EXTENSION

#include "breach/browser/node/api/browser_wrap.h"

#include "base/command_line.h"
#include "net/base/net_util.h"
#include "content/public/browser/browser_thread.h"
#include "breach/browser/ui/browser.h"
#include "breach/browser/breach_content_browser_client.h"
#include "breach/browser/breach_browser_context.h"

using namespace v8;

namespace breach {

/* TODO(spolu) Code check */
static GURL GetStartupURL() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  const CommandLine::StringVector& args = command_line->GetArgs();

  if (args.empty())
    return GURL("http://www.google.com/");

  GURL url(args[0]);
  if (url.is_valid() && url.has_scheme())
    return url;

  return net::FilePathToFileURL(base::FilePath(args[0]));
}

BrowserWrap::BrowserWrap()
{
  this->AddRef();
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&BrowserWrap::CreateBrowser, this));

}

BrowserWrap::~BrowserWrap()
{
  this->Release();
}

void
BrowserWrap::CreateBrowser()
{
  browser_.reset(Browser::CreateNewWindow(
                   BreachContentBrowserClient::Get()->browser_context(),
                   GetStartupURL(),
                   NULL,
                   MSG_ROUTING_NONE,
                   gfx::Size()));
}

void 
BrowserWrap::New(
    const v8::FunctionCallbackInfo<v8::Value>& args)
{
  HandleScope handle_scope(Isolate::GetCurrent());
  BrowserWrap* browser_wrap = new BrowserWrap();
  browser_wrap->Wrap(args.This());

  args.GetReturnValue().Set(args.This());
}

void 
BrowserWrap::Init(
    Handle<Object> exports) 
{
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Browser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  //tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
  //    FunctionTemplate::New(PlusOne)->GetFunction());

  exports->Set(String::NewSymbol("Browser"), tpl->GetFunction());
}
    
} // namespace breach

