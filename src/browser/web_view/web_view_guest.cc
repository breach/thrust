// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/browser/web_view/web_view_guest.h"

#include "base/lazy_instance.h"
#include "net/base/escape.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "content/public/browser/host_zoom_map.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/resource_request_details.h"
#include "content/public/common/page_zoom.h"

#include "src/browser/web_view/web_view_constants.h"

using content::WebContents;

namespace {

// <embedder_process_id, guest_instance_id> => WebViewGuest*
typedef std::map<std::pair<int, int>, WebViewGuest*> EmbedderWebViewGuestMap;
static base::LazyInstance<EmbedderWebViewGuestMap> embedder_webview_map =
    LAZY_INSTANCE_INITIALIZER;

// WebContents* => WebViewGuest*
typedef std::map<WebContents*, WebViewGuest*> WebContentsWebViewGuestMap;
static base::LazyInstance<WebContentsWebViewGuestMap> webcontents_webview_map =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

WebViewGuest::Event::Event(
    const std::string& name,
    scoped_ptr<base::DictionaryValue> args)
  : name_(name),
    args_(args.Pass()) 
{
}

WebViewGuest::Event::~Event() 
{
}

scoped_ptr<base::DictionaryValue> 
WebViewGuest::Event::GetArguments() 
{
  return args_.Pass();
}

WebViewGuest::WebViewGuest(
    int guest_instance_id,
    WebContents* guest_web_contents)
: WebContentsObserver(guest_web_contents),
  guest_web_contents_(guest_web_contents),
  embedder_web_contents_(NULL),
  embedder_render_process_id_(0),
  browser_context_(guest_web_contents->GetBrowserContext()),
  guest_instance_id_(guest_instance_id),
  view_instance_id_(webview::kInstanceIDNone),
  weak_ptr_factory_(this) 
{
  webcontents_webview_map.Get().insert(
      std::make_pair(guest_web_contents, this));

  notification_registrar_.Add(
      this, content::NOTIFICATION_LOAD_COMPLETED_MAIN_FRAME,
      content::Source<WebContents>(guest_web_contents));

  notification_registrar_.Add(
      this, content::NOTIFICATION_RESOURCE_RECEIVED_REDIRECT,
      content::Source<WebContents>(guest_web_contents));
}

// static
WebViewGuest* 
WebViewGuest::Create(
    int guest_instance_id,
    WebContents* guest_web_contents)
{
  return new WebViewGuest(guest_instance_id, guest_web_contents);
}

// static
WebViewGuest* 
WebViewGuest::FromWebContents(
    WebContents* web_contents) 
{
  WebContentsWebViewGuestMap* webview_map = webcontents_webview_map.Pointer();
  WebContentsWebViewGuestMap::iterator it = webview_map->find(web_contents);
  return it == webview_map->end() ? NULL : it->second;
}

// static
WebViewGuest* 
WebViewGuest::From(
    int embedder_process_id, 
    int guest_instance_id) 
{
  EmbedderWebViewGuestMap* guest_map = embedder_webview_map.Pointer();
  EmbedderWebViewGuestMap::iterator it = guest_map->find(
      std::make_pair(embedder_process_id, guest_instance_id));
  return it == guest_map->end() ? NULL : it->second;
}

// static.
int 
WebViewGuest::GetViewInstanceId(
    WebContents* contents) 
{
  WebViewGuest* guest = FromWebContents(contents);
  if(!guest)
    return webview::kInstanceIDNone;

  return guest->view_instance_id();
}

void 
WebViewGuest::Attach(
    content::WebContents* embedder_web_contents,
    const base::DictionaryValue& args) 
{
  embedder_web_contents_ = embedder_web_contents;
  embedder_render_process_id_ =
      embedder_web_contents->GetRenderProcessHost()->GetID();
  args.GetInteger(webview::kParameterInstanceId, &view_instance_id_);

  std::pair<int, int> key(embedder_render_process_id_, guest_instance_id_);
  embedder_webview_map.Get().insert(std::make_pair(key, this));

  // WebViewGuest::Attach is called prior to initialization (and initial
  // navigation) of the guest in the content layer in order to permit mapping
  // the necessary associations between the <*view> element and its guest. This
  // is needed by the <webview> WebRequest API to allow intercepting resource
  // requests during navigation. However, queued events should be fired after
  // content layer initialization in order to ensure that load events (such as
  // 'loadstop') fire in embedder after the contentWindow is available.
  /* TODO(spolu): Check if should return */
  /*
  if (!in_extension())
    return;
  */

  base::MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&WebViewGuest::SendQueuedEvents,
                 weak_ptr_factory_.GetWeakPtr()));
}

WebViewGuest::~WebViewGuest() 
{
  std::pair<int, int> key(embedder_render_process_id_, guest_instance_id_);
  embedder_webview_map.Get().erase(key);

  webcontents_webview_map.Get().erase(guest_web_contents());

  while (!pending_events_.empty()) {
    delete pending_events_.front();
    pending_events_.pop();
  }
}

void 
WebViewGuest::Observe(int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) 
{
  switch (type) {
    case content::NOTIFICATION_LOAD_COMPLETED_MAIN_FRAME: {
      DCHECK_EQ(content::Source<WebContents>(source).ptr(),
                guest_web_contents());
      if(content::Source<WebContents>(source).ptr() == guest_web_contents()) {
        //LoadHandlerCalled();
      }
      break;
    }
    case content::NOTIFICATION_RESOURCE_RECEIVED_REDIRECT: {
      DCHECK_EQ(content::Source<WebContents>(source).ptr(),
                guest_web_contents());
      content::ResourceRedirectDetails* resource_redirect_details =
          content::Details<content::ResourceRedirectDetails>(details).ptr();
      bool is_top_level =
          resource_redirect_details->resource_type == ResourceType::MAIN_FRAME;
      /*
      LoadRedirect(resource_redirect_details->url,
                   resource_redirect_details->new_url,
                   is_top_level);
      */
      break;
    }
    default:
      NOTREACHED() << "Unexpected notification sent.";
      break;
  }
}

void 
WebViewGuest::SetZoom(
    double zoom_factor) 
{
  double zoom_level = content::ZoomFactorToZoomLevel(zoom_factor);
  content::HostZoomMap::SetZoomLevel(guest_web_contents(), zoom_level);

  /*
  scoped_ptr<base::DictionaryValue> args(new base::DictionaryValue());
  args->SetDouble(webview::kOldZoomFactor, current_zoom_factor_);
  args->SetDouble(webview::kNewZoomFactor, zoom_factor);
  DispatchEvent(
      new GuestViewBase::Event(webview::kEventZoomChange, args.Pass()));
  */

  current_zoom_factor_ = zoom_factor;
}

double 
WebViewGuest::GetZoom() 
{
  return current_zoom_factor_;
}

void 
WebViewGuest::Go(
    int relative_index) 
{
  guest_web_contents()->GetController().GoToOffset(relative_index);
}

void 
WebViewGuest::Reload() 
{
  // TODO(fsamuel): Don't check for repost because we don't want to show
  // Chromium's repost warning. We might want to implement a separate API
  // for registering a callback if a repost is about to happen.
  guest_web_contents()->GetController().Reload(false);
}

void 
WebViewGuest::Stop() 
{
  guest_web_contents()->Stop();
}


void 
WebViewGuest::DispatchEvent(
    Event* event) 
{
  if (!attached()) {
    pending_events_.push(event);
    return;
  }

  /*
  Profile* profile = Profile::FromBrowserContext(browser_context_);

  extensions::EventFilteringInfo info;
  info.SetURL(GURL());
  info.SetInstanceID(guest_instance_id_);
  scoped_ptr<base::ListValue> args(new base::ListValue());
  args->Append(event->GetArguments().release());

  extensions::EventRouter::DispatchEvent(
      embedder_web_contents_, profile, embedder_extension_id_,
      event->name(), args.Pass(),
      extensions::EventRouter::USER_GESTURE_UNKNOWN, info);
  */

  delete event;
}

void 
WebViewGuest::SendQueuedEvents() 
{
  if (!attached())
    return;

  while (!pending_events_.empty()) {
    Event* event = pending_events_.front();
    pending_events_.pop();
    DispatchEvent(event);
  }
}
