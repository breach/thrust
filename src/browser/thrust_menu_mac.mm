// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#import "src/browser/thrust_menu.h"

#import "base/mac/scoped_nsobject.h"
#import "base/strings/sys_string_conversions.h"

#import "src/browser/ui/cocoa/menu_controller.h"
#import "src/browser/thrust_window.h"

namespace thrust_shell {

static base::scoped_nsobject<ThrustShellMenuController> menu_controller_;
static ThrustMenu*                                      application_menu_ = NULL;

void 
ThrustMenu::PlatformPopup(ThrustWindow* window) {
  base::scoped_nsobject<ThrustShellMenuController> menu_controller(
      [[ThrustShellMenuController alloc] initWithModel:model_.get()]);

  NSWindow* nswindow = window->GetNativeWindow();
  content::WebContents* web_contents = window->GetWebContents();

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

void
ThrustMenu::PlatformCleanup()
{
  if(application_menu_ == this) {
    [NSApp setMainMenu: nil];
    application_menu_ = NULL;
  }
}

// static
void 
ThrustMenu::SetApplicationMenu(ThrustMenu* menu) {
  base::scoped_nsobject<ThrustShellMenuController> menu_controller(
      [[ThrustShellMenuController alloc] initWithModel:menu->model_.get()]);
  [NSApp setMainMenu:[menu_controller menu]];

  /* Ensure the menu_controller_ is destroyed after main menu is set. */
  menu_controller.swap(menu_controller_);
  /* We store the menu in a static pointer to make sure to clean after */
  /* ourselves when the menu get destroyed (see PlatformCleanup)       */
  application_menu_ = menu;
}

// static
void 
ThrustMenu::SendActionToFirstResponder(const std::string& action) {
  SEL selector = NSSelectorFromString(base::SysUTF8ToNSString(action));
  [NSApp sendAction:selector to:nil from:[NSApp mainMenu]];
}

} // namespace thrust_shell
