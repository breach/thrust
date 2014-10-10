// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/exo_menu.h"

using namespace content;

namespace exo_shell {

void 
ExoMenu::PlatformPopup(ExoShell* shell) {
  base::scoped_nsobject<MenuController> menu_controller(
      [[MenuController alloc] initWithModel:model_.get()]);

  NSWindow* nswindow = shell->GetNativeWindow();
  content::WebContents* web_contents = shell->GetWebContents();

  // Fake out a context menu event.
  NSEvent* currentEvent = [NSApp currentEvent];
  NSPoint position = [nswindow mouseLocationOutsideOfEventStream];
  NSTimeInterval eventTime = [currentEvent timestamp];
  NSEvent* clickEvent = [NSEvent mouseEventWithType:NSRightMouseDown
                                           location:position
                                      modifierFlags:NSRightMouseDownMask
                                          timestamp:eventTime
                                       windowNumber:[nswindow windowNumber]
                                            context:nil
                                        eventNumber:0
                                         clickCount:1
                                           pressure:1.0];

  // Show the menu.
  [NSMenu popUpContextMenu:[menu_controller menu]
                 withEvent:clickEvent
                   forView:web_contents->GetContentNativeView()];
}

// static
void 
ExoMenu::SetApplicationMenu(Menu* base_menu) {
  MenuMac* menu = static_cast<MenuMac*>(base_menu);
  base::scoped_nsobject<AtomMenuController> menu_controller(
      [[AtomMenuController alloc] initWithModel:menu->model_.get()]);
  [NSApp setMainMenu:[menu_controller menu]];

  // Ensure the menu_controller_ is destroyed after main menu is set.
  menu_controller.swap(menu->menu_controller_);
}

// static
void 
ExoMenu::SendActionToFirstResponder(const std::string& action) {
  SEL selector = NSSelectorFromString(base::SysUTF8ToNSString(action));
  [NSApp sendAction:selector to:nil from:[NSApp mainMenu]];
}

} // namespace exo_shell
