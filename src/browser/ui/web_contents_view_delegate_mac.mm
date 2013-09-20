// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/ui/web_contents_view_delegate.h"

#import  <Cocoa/Cocoa.h>

#include "base/command_line.h"
#include "third_party/WebKit/public/web/WebContextMenuData.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_widget_host_view.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/context_menu_params.h"

using namespace content;

using WebKit::WebContextMenuData;

enum {
  ExoBrowserContextMenuItemCutTag = 0,
  ExoBrowserContextMenuItemCopyTag,
  ExoBrowserContextMenuItemPasteTag,
  ExoBrowserContextMenuItemDeleteTag,
  ExoBrowserContextMenuItemBackTag,
  ExoBrowserContextMenuItemForwardTag,
  ExoBrowserContextMenuItemReloadTag,
  ExoBrowserContextMenuItemInspectTag
};


@interface ExoBrowserContextMenuDelegate : NSObject<NSMenuDelegate> {
 @private
  exo_browser::ExoBrowserWebContentsViewDelegate* delegate_;
}
@end

@implementation ExoBrowserContextMenuDelegate
- (id)initWithDelegate:
    (exo_browser::ExoBrowserWebContentsViewDelegate*) delegate {
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
    ExoBrowserContextMenuDelegate* delegate) 
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
}

void 
ExoBrowserWebContentsViewDelegate::ShowContextMenu(
    const ContextMenuParams& params) 
{
  params_ = params;
  bool has_link = !params_.unfiltered_link_url.is_empty();
  bool has_selection = ! params_.selection_text.empty();

  NSMenu* menu = [[[NSMenu alloc] initWithTitle:@""] autorelease];
  ExoBrowserContextMenuDelegate* delegate =
      [[ExoBrowserContextMenuDelegate alloc] initWithDelegate:this];
  [menu setDelegate:delegate];
  [menu setAutoenablesItems:NO];

  if (params.media_type == WebContextMenuData::MediaTypeNone &&
      !has_link &&
      !has_selection &&
      !params_.is_editable) {
    BOOL back_menu_enabled =
        web_contents_->GetController().CanGoBack() ? YES : NO;
    MakeContextMenuItem(@"Back",
                        ExoBrowserContextMenuItemBackTag,
                        menu,
                        back_menu_enabled,
                        delegate);

    BOOL forward_menu_enabled =
        web_contents_->GetController().CanGoForward() ? YES : NO;
    MakeContextMenuItem(@"Forward",
                        ExoBrowserContextMenuItemForwardTag,
                        menu,
                        forward_menu_enabled,
                        delegate);

    MakeContextMenuItem(@"Reload",
                        ExoBrowserContextMenuItemReloadTag,
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
                        ExoBrowserContextMenuItemCutTag,
                        menu,
                        cut_menu_enabled,
                        delegate);

    BOOL copy_menu_enabled =
        (params_.edit_flags & WebContextMenuData::CanCopy) ? YES : NO;
    MakeContextMenuItem(@"Copy",
                        ExoBrowserContextMenuItemCopyTag,
                        menu,
                        copy_menu_enabled,
                        delegate);

    BOOL paste_menu_enabled =
        (params_.edit_flags & WebContextMenuData::CanPaste) ? YES : NO;
    MakeContextMenuItem(@"Paste",
                        ExoBrowserContextMenuItemPasteTag,
                        menu,
                        paste_menu_enabled,
                        delegate);

    BOOL delete_menu_enabled =
        (params_.edit_flags & WebContextMenuData::CanDelete) ? YES : NO;
    MakeContextMenuItem(@"Delete",
                        ExoBrowserContextMenuItemDeleteTag,
                        menu,
                        delete_menu_enabled,
                        delegate);

    NSMenuItem* separator = [NSMenuItem separatorItem];
    [menu addItem:separator];
  } 
  else if (has_selection) {
    MakeContextMenuItem(@"Copy",
                        ExoBrowserContextMenuItemCopyTag,
                        menu,
                        YES,
                        delegate);

    NSMenuItem* separator = [NSMenuItem separatorItem];
    [menu addItem:separator];
  }

  MakeContextMenuItem(@"Inspect",
                      ExoBrowserContextMenuItemInspectTag,
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
ExoBrowserWebContentsViewDelegate::ActionPerformed(
    int tag) 
{
  switch (tag) {
    case ExoBrowserContextMenuItemCutTag:
      web_contents_->GetRenderViewHost()->Cut();
      break;
    case ExoBrowserContextMenuItemCopyTag:
      web_contents_->GetRenderViewHost()->Copy();
      break;
    case ExoBrowserContextMenuItemPasteTag:
      web_contents_->GetRenderViewHost()->Paste();
      break;
    case ExoBrowserContextMenuItemDeleteTag:
      web_contents_->GetRenderViewHost()->Delete();
      break;
    case ExoBrowserContextMenuItemBackTag:
      web_contents_->GetController().GoToOffset(-1);
      web_contents_->GetView()->Focus();
      break;
    case ExoBrowserContextMenuItemForwardTag:
      web_contents_->GetController().GoToOffset(1);
      web_contents_->GetView()->Focus();
      break;
    case ExoBrowserContextMenuItemReloadTag: {
      web_contents_->GetController().Reload(false);
      web_contents_->GetView()->Focus();
      break;
    }
    case ExoBrowserContextMenuItemInspectTag: {
      /* TODO(spolu): Handle when DevTools are implemented */
      // ExoBrowserDevToolsFrontend::Show(web_contents_);
      break;
    }
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
