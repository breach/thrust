// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/renderer/extensions/context.h"

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_split.h"
#include "base/values.h"
#include "content/public/renderer/render_view.h"
#include "content/public/renderer/v8_value_converter.h"
#include "third_party/WebKit/public/web/WebFrame.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebScopedMicrotaskSuppression.h"
#include "third_party/WebKit/public/web/WebView.h"
#include "v8/include/v8.h"

#include "src/renderer/extensions/module_system.h"

using content::V8ValueConverter;

namespace extensions {

Context::Context(v8::Handle<v8::Context> v8_context,
                 blink::WebFrame* web_frame)
    : v8_context_(v8_context),
      web_frame_(web_frame),
      safe_builtins_(this),
      isolate_(v8_context->GetIsolate()) {
  VLOG(1) << "Created context:\n"
          << "  frame:        " << web_frame_;
}

Context::~Context() {
  VLOG(1) << "Destroyed context:\n"
          << "  frame:        " << web_frame_;
  Invalidate();
}

void Context::Invalidate() {
  if (!is_valid())
    return;
  if (module_system_)
    module_system_->Invalidate();
  web_frame_ = NULL;
  v8_context_.reset();
}

content::RenderView* Context::GetRenderView() const {
  if (web_frame_ && web_frame_->view())
    return content::RenderView::FromWebView(web_frame_->view());
  else
    return NULL;
}

GURL Context::GetURL() const {
  /* TODO(spolu): Check OK                      */
  /*              see: user_script_slave.cc:216 */
  return web_frame_ ?
    GURL(web_frame_->document().url().string()) : GURL();
}

v8::Local<v8::Value> Context::CallFunction(
    v8::Handle<v8::Function> function,
    int argc,
    v8::Handle<v8::Value> argv[]) const {
  v8::EscapableHandleScope handle_scope(isolate());
  v8::Context::Scope scope(v8_context());

  blink::WebScopedMicrotaskSuppression suppression;
  if (!is_valid()) {
    return handle_scope.Escape(
        v8::Local<v8::Primitive>(v8::Undefined(isolate())));
  }

  v8::Handle<v8::Object> global = v8_context()->Global();
  if (!web_frame_)
    return handle_scope.Escape(function->Call(global, argc, argv));
  return handle_scope.Escape(
      v8::Local<v8::Value>(web_frame_->callFunctionEvenIfScriptDisabled(
          function, global, argc, argv)));
}

}  // namespace extensions
