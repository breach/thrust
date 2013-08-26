// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/ui/breach_web_contents_view_delegate.h"

#import  <Cocoa/Cocoa.h>

#include "base/command_line.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/context_menu_params.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"

using namespace content;

using WebKit::WebContextMenuData;

enum {
  BreachContextMenuItemCutTag = 0,
  BreachContextMenuItemCopyTag,
  BreachContextMenuItemPasteTag,
  BreachContextMenuItemDeleteTag,
  BreachContextMenuItemOpenLinkTag,
  BreachContextMenuItemBackTag,
  BreachContextMenuItemForwardTag,
  BreachContextMenuItemReloadTag,
  BreachContextMenuItemInspectTag
};


@interface BreachContextMenuDelegate : NSObject<NSMenuDelegate> {
 @private
  breach::BreachWebContentsViewDelegate* delegate_;
}
@end

@implementation BreachContextMenuDelegate
- (id)initWithDelegate:(breach::BreachWebContentsViewDelegate*) delegate {
  if ((self = [super init])) {
    delegate_ = delegate;
  }
  return self;
}

- (void)itemSelected:(id)sender {
  NSInteger tag = [sender tag];
  delegate_->ActionPerformed(tag);
}
@end

namespace {

NSMenuItem* 
MakeContextMenuItem(
    NSString* title,
    NSInteger tag,
    NSMenu* menu,
    BOOL enabled,
    BreachContextMenuDelegate* delegate) 
{
  NSMenuItem* menu_item =
      [[NSMenuItem alloc] initWithTitle:title
                                 action:@selector(itemSelected:)
                          keyEquivalent:@""];
  [menu_item setTarget:delegate];
  [menu_item setTag:tag];
  [menu_item setEnabled:enabled];
  [menu addItem:menu_item];

  return menu_item;
}

}  // namespace

namespace breach {

WebContentsViewDelegate* 
CreateBreachWebContentsViewDelegate(
    WebContents* web_contents) {
  return new BreachWebContentsViewDelegate(web_contents);
}

BreachWebContentsViewDelegate::BreachWebContentsViewDelegate(
    WebContents* web_contents)
    : web_contents_(web_contents) 
{
}

BreachWebContentsViewDelegate::~BreachWebContentsViewDelegate() 
{
}

void 
BreachWebContentsViewDelegate::ShowContextMenu(
    const ContextMenuParams& params) 
{
  params_ = params;
  bool has_link = !params_.unfiltered_link_url.is_empty();
  bool has_selection = ! params_.selection_text.empty();

  NSMenu* menu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
  BreachContextMenuDelegate* delegate =
      [[BreachContextMenuDelegate alloc] initWithDelegate:this];
  [menu setDelegate:delegate];
  [menu setAutoenablesItems:NO];

  if (params.media_type == WebContextMenuData::MediaTypeNone &&
      !has_link &&
      !has_selection &&
      !params_.is_editable) {
    BOOL back_menu_enabled =
        web_contents_->GetController().CanGoBack() ? YES : NO;
    MakeContextMenuItem(@"Back",
                        BreachContextMenuItemBackTag,
                        menu,
                        back_menu_enabled,
                        delegate);

    BOOL forward_menu_enabled =
        web_contents_->GetController().CanGoForward() ? YES : NO;
    MakeContextMenuItem(@"Forward",
                        BreachContextMenuItemForwardTag,
                        menu,
                        forward_menu_enabled,
                        delegate);

    MakeContextMenuItem(@"Reload",
                        BreachContextMenuItemReloadTag,
                        menu,
                        YES,
                        delegate);

    NSMenuItem* separator = [NSMenuItem separatorItem];
    [menu addItem:separator];
  }

  if (has_link) {
    MakeContextMenuItem(@"Open Link in New Tab",
                        BreachContextMenuItemOpenLinkTag,
                        menu,
                        YES,
                        delegate);

    NSMenuItem* separator = [NSMenuItem separatorItem];
    [menu addItem:separator];
  }

  if (params_.is_editable) {
    BOOL cut_menu_enabled =
        (params_.edit_flags & WebContextMenuData::CanCut) ? YES : NO;
    MakeContextMenuItem(@"Cut",
                        BreachContextMenuItemCutTag,
                        menu,
                        cut_menu_enabled,
                        delegate);

    BOOL copy_menu_enabled =
        (params_.edit_flags & WebContextMenuData::CanCopy) ? YES : NO;
    MakeContextMenuItem(@"Copy",
                        BreachContextMenuItemCopyTag,
                        menu,
                        copy_menu_enabled,
                        delegate);

    BOOL paste_menu_enabled =
        (params_.edit_flags & WebContextMenuData::CanPaste) ? YES : NO;
    MakeContextMenuItem(@"Paste",
                        BreachContextMenuItemPasteTag,
                        menu,
                        paste_menu_enabled,
                        delegate);

    BOOL delete_menu_enabled =
        (params_.edit_flags & WebContextMenuData::CanDelete) ? YES : NO;
    MakeContextMenuItem(@"Delete",
                        BreachContextMenuItemDeleteTag,
                        menu,
                        delete_menu_enabled,
                        delegate);

    NSMenuItem* separator = [NSMenuItem separatorItem];
    [menu addItem:separator];
  } 
  else if (has_selection) {
    MakeContextMenuItem(@"Copy",
                        BreachContextMenuItemCopyTag,
                        menu,
                        YES,
                        delegate);

    NSMenuItem* separator = [NSMenuItem separatorItem];
    [menu addItem:separator];
  }

  MakeContextMenuItem(@"Inspect",
                      BreachContextMenuItemInspectTag,
                      menu,
                      YES,
                      delegate);

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

void 
BreachWebContentsViewDelegate::ActionPerformed(
    int tag) 
{
  switch (tag) {
    case BreachContextMenuItemCutTag:
      web_contents_->GetRenderViewHost()->Cut();
      break;
    case BreachContextMenuItemCopyTag:
      web_contents_->GetRenderViewHost()->Copy();
      break;
    case BreachContextMenuItemPasteTag:
      web_contents_->GetRenderViewHost()->Paste();
      break;
    case BreachContextMenuItemDeleteTag:
      web_contents_->GetRenderViewHost()->Delete();
      break;
    case BreachContextMenuItemOpenLinkTag: {
      /* TODO(spolu): Handle */
      /*
      ShellBrowserContext* browser_context =
          ShellContentBrowserClient::Get()->browser_context();
      Shell::CreateNewWindow(browser_context,
                             params_.link_url,
                             NULL,
                             MSG_ROUTING_NONE,
                             gfx::Size()); 
      */
      break;
    }
    case BreachContextMenuItemBackTag:
      web_contents_->GetController().GoToOffset(-1);
      web_contents_->GetView()->Focus();
      break;
    case BreachContextMenuItemForwardTag:
      web_contents_->GetController().GoToOffset(1);
      web_contents_->GetView()->Focus();
      break;
    case BreachContextMenuItemReloadTag: {
      web_contents_->GetController().Reload(false);
      web_contents_->GetView()->Focus();
      break;
    }
    case BreachContextMenuItemInspectTag: {
      /* TODO(spolu): Handle when DevTools are implemented */
      // BreachDevToolsFrontend::Show(web_contents_);
      break;
    }
  }
}

WebDragDestDelegate* 
BreachWebContentsViewDelegate::GetDragDestDelegate() 
{
  return NULL;
}

NSObject<RenderWidgetHostViewMacDelegate>*
BreachWebContentsViewDelegate::CreateRenderWidgetHostViewDelegate(
    content::RenderWidgetHost* render_widget_host) 
{
  return NULL;
}

} // namespace breach
