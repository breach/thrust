// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/exo_frame.h"

#include "base/logging.h"
#import "base/mac/scoped_nsobject.h"
#include "base/strings/string_piece.h"
#include "base/strings/sys_string_conversions.h"
#include "url/gurl.h"
#import "ui/base/cocoa/underlay_opengl_hosting_window.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"

using namespace content;

namespace exo_browser {

gfx::Size
ExoFrame::PlatformSize()
{
  WebContentsView* content_view = web_contents_->GetView();
  NSRect frame = [content_view->GetNativeView() frame];
  return gfx::Size(frame.size.width, frame.size.height);
}

} // namespace exo_browser

