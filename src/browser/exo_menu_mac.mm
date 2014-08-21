// Copyright (c) 2013 Stanislas Polu.
// See the LICENSE file.

#include "src/browser/exo_menu_mac.h"
#include <algorithm>

#include "base/logging.h"
#import "base/mac/scoped_nsobject.h"
#include "base/strings/string_piece.h"
#include "base/strings/sys_string_conversions.h"
#include "url/gurl.h"
#import "ui/base/cocoa/underlay_opengl_hosting_window.h"
#include "content/public/browser/native_web_keyboard_event.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "vendor/brightray/browser/inspectable_web_contents_view.h"


using namespace content;


namespace exo_shell {

ExoMenuMac::ExoMenuMac() {
}

void 
ExoMenuMac::Popup() {
  // base::scoped_nsobject<ExoMenuController> menu_controller(
  //     [[ExoMenuController alloc] initWithModel:model_.get()]);

  // NativeWindow* native_window = window->window();
  // NSWindow* nswindow = native_window->GetNativeWindow();
  // content::WebContents* web_contents = native_window->GetWebContents();

  // // Fake out a context menu event.
  // NSEvent* currentEvent = [NSApp currentEvent];
  // NSPoint position = [nswindow mouseLocationOutsideOfEventStream];
  // NSTimeInterval eventTime = [currentEvent timestamp];
  // NSEvent* clickEvent = [NSEvent mouseEventWithType:NSRightMouseDown
  //                                          location:position
  //                                     modifierFlags:NSRightMouseDownMask
  //                                         timestamp:eventTime
  //                                      windowNumber:[nswindow windowNumber]
  //                                           context:nil
  //                                       eventNumber:0
  //                                        clickCount:1
  //                                          pressure:1.0];

  // // Show the menu.
  // [NSMenu popUpContextMenu:[menu_controller menu]
  //                withEvent:clickEvent
  //                  forView:web_contents->GetContentNativeView()];
}




void 
ExoMenu::SetApplicationMenu(ExoMenu* base_menu) 
{
  ExoMenuMac* menu = static_cast<ExoMenuMac*>(base_menu);
  base::scoped_nsobject<ExoMenuController> menu_controller(
      [[ExoMenuController alloc] initWithModel:menu->model_.get()]);
  
  [NSApp setMainMenu:[menu_controller menu]];
  //menu->Popup();
  // Ensure the menu_controller_ is destroyed after main menu is set.
  //menu_controller.swap(menu->menu_controller_);

   NSApplication *app = [NSApplication sharedApplication];
  [app setMainMenu:[menu_controller menu]];
   
   NSMenu *mainMenu = [[NSMenu alloc] initWithTitle: @"MainMenu"];
    NSMenuItem *mi;
    NSMenu *m;
    mi = [mainMenu addItemWithTitle:@"Apple" action:NULL
keyEquivalent:@""];
    m = [[NSMenu alloc] initWithTitle:@"Apple"];
    // strange hack
    //[NSApp performSelector:@selector(setAppleMenu:) withObject: m];
    /*[mainMenu setSubmenu:m forItem:mi];
    mi = [m addItemWithTitle: @"Test Item"
            action: nil
            keyEquivalent: @""];*/

    [app setMainMenu: mainMenu];

   
}

void
ExoMenu::SendActionToFirstResponder(const std::string& action) {
  SEL selector = NSSelectorFromString(base::SysUTF8ToNSString(action));
  [NSApp sendAction:selector to:nil from:[NSApp mainMenu]];
}


} // namespace exo_shell

