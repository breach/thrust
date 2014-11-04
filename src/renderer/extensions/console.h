// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_RENDERER_CONSOLE_H_
#define EXTENSIONS_RENDERER_CONSOLE_H_

#include <string>

#include "content/public/common/console_message_level.h"
#include "v8/include/v8.h"

namespace content {
class RenderView;
}

namespace extensions {

// Utility for logging messages to RenderViews.
namespace console {

// Adds |message| to the console of |render_view| at various log levels.
void Debug(content::RenderView* render_view, const std::string& message);
void Log(content::RenderView* render_view, const std::string& message);
void Warn(content::RenderView* render_view, const std::string& message);
void Error(content::RenderView* render_view, const std::string& message);

// Logs an Error then crashes the current process.
void Fatal(content::RenderView* render_view, const std::string& message);

void AddMessage(content::RenderView* render_view,
                content::ConsoleMessageLevel level,
                const std::string& message);

// Adds |message| to the console that hosts |context|, if any.
void Debug(v8::Handle<v8::Context> context, const std::string& message);
void Log(v8::Handle<v8::Context> context, const std::string& message);
void Warn(v8::Handle<v8::Context> context, const std::string& message);
void Error(v8::Handle<v8::Context> context, const std::string& message);

// Logs an Error then crashes the current process.
void Fatal(v8::Handle<v8::Context> context, const std::string& message);

void AddMessage(v8::Handle<v8::Context> context,
                content::ConsoleMessageLevel level,
                const std::string& message);

// Returns a new v8::Object with each standard log method (Debug/Log/Warn/Error)
// bound to respective debug/log/warn/error methods. This is a direct drop-in
// replacement for the standard devtools console.* methods usually accessible
// from JS.
v8::Local<v8::Object> AsV8Object();

}  // namespace console

}  // namespace extensions

#endif  // EXTENSIONS_RENDERER_CONSOLE_H_
