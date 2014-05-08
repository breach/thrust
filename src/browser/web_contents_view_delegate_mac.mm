// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/web_contents_view_delegate.h"

#import  <Cocoa/Cocoa.h>

#include "base/command_line.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/context_menu_params.h"
#include "exo_browser/src/node/node_thread.h"
#include "exo_browser/src/browser/exo_frame.h"
#include "exo_browser/src/node/api/exo_frame_wrap.h"

using namespace content;

using blink::WebContextMenuData;


@interface ExoBrowserContextMenuDelegate : NSObject<NSMenuDelegate> {
 @private
  exo_browser::ExoBrowserWebContentsViewDelegate::Executor* delegate_;
}
@end

@implementation ExoBrowserContextMenuDelegate
- (id)initWithDelegate:
    (exo_browser::ExoBrowserWebContentsViewDelegate::Executor*) delegate {
  if ((self = [super init])) {
    delegate_ = delegate;
  }
  return self;
}

- (void)itemSelected:(id)sender {
  NSInteger index = [sender tag];
  delegate_->ActionPerformed(index);
}
@end

namespace {

NSMenuItem* 
MakeContextMenuItem(
    NSString* title,
    NSInteger index,
    NSMenu* menu,
    BOOL enabled,
    ExoBrowserContextMenuDelegate* delegate) 
{
  NSMenuItem* menu_item =
      [[NSMenuItem alloc] initWithTitle:title
                                 action:@selector(itemSelected:)
                          keyEquivalent:@""];
  [menu_item setTarget:delegate];
  [menu_item setTag:index];
  [menu_item setEnabled:enabled];
  [menu addItem:menu_item];

  return menu_item;
}

}  // namespace

namespace exo_browser {

WebContentsViewDelegate* 
CreateExoBrowserWebContentsViewDelegate(
    WebContents* web_contents) {
  return new ExoBrowserWebContentsViewDelegate(web_contents);
}

ExoBrowserWebContentsViewDelegate::ExoBrowserWebContentsViewDelegate(
    WebContents* web_contents)
    : web_contents_(web_contents) 
{
}

ExoBrowserWebContentsViewDelegate::~ExoBrowserWebContentsViewDelegate() 
{
  LOG(INFO) << "ExoBrowserWebContentsViewDelegate Destructor";
}

void 
ExoBrowserWebContentsViewDelegate::ShowContextMenu(
    RenderFrameHost* render_frame_host,
    const ContextMenuParams& params) 
{
  base::Callback<void(const std::vector<std::string>&, 
                      const base::Callback<void(const int)>&)> callback = 
    base::Bind(&ExoBrowserWebContentsViewDelegate::Executor::BuildContextMenu, 
               new Executor(web_contents_));

  ExoFrame* exo_frame = ExoFrame::ExoFrameForWebContents(web_contents_);
  if(exo_frame && exo_frame->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::CallBuildContextMenu, 
                   exo_frame->wrapper_, params, callback));
  }
}


void
ExoBrowserWebContentsViewDelegate::Executor::BuildContextMenu(
    const std::vector<std::string>& items,
    const base::Callback<void(const int)>& trigger)
{
  if(items.size() > 0) {
    NSMenu* menu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
    ExoBrowserContextMenuDelegate* delegate =
      [[ExoBrowserContextMenuDelegate alloc] initWithDelegate:this];
    [menu setDelegate:delegate];
    [menu setAutoenablesItems:NO];

    for(unsigned int i = 0; i < items.size(); i ++) {
      NSString *item = [NSString stringWithUTF8String:items[i].c_str()];

      if([item length] == 0) {
        NSMenuItem* separator = [NSMenuItem separatorItem];
        [menu addItem:separator];
      }
      else {
        // BOOL back_menu_enabled =
        //    web_contents_->GetController().CanGoBack() ? YES : NO;
        MakeContextMenuItem(item, i, menu, YES, delegate);
      }
    }

    NSView* parent_view = web_contents_->GetView()->GetContentNativeView();
    NSEvent* currentEvent = [NSApp currentEvent];
    NSWindow* window = [parent_view window];
    NSPoint position = [window mouseLocationOutsideOfEventStream];
    NSTimeInterval eventTime = [currentEvent timestamp];
    NSEvent* clickEvent = [NSEvent mouseEventWithType:NSRightMouseDown
                                             location:position
                                        modifierFlags:NSRightMouseDownMask
                                            timestamp:eventTime
                                         windowNumber:[window windowNumber]
                                              context:nil
                                          eventNumber:0
                                           clickCount:1
                                             pressure:1.0];

    [NSMenu popUpContextMenu:menu
                   withEvent:clickEvent
                     forView:parent_view];

  }
}

void 
ExoBrowserWebContentsViewDelegate::Executor::ActionPerformed(
    int index) 
{
  ExoFrame* exo_frame = ExoFrame::ExoFrameForWebContents(web_contents_);
  if(exo_frame && exo_frame->wrapper_) {
    NodeThread::Get()->PostTask(
        FROM_HERE,
        base::Bind(&ExoFrameWrap::CallTriggerContextMenuItem, 
                   exo_frame->wrapper_, index));
  }
}

WebDragDestDelegate* 
ExoBrowserWebContentsViewDelegate::GetDragDestDelegate() 
{
  return NULL;
}

NSObject<RenderWidgetHostViewMacDelegate>*
ExoBrowserWebContentsViewDelegate::CreateRenderWidgetHostViewDelegate(
    content::RenderWidgetHost* render_widget_host) 
{
  return NULL;
}

} // namespace exo_browser
