// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo/exo_browser/browser/ui/dialog/javascript_dialog.h"

#import <Cocoa/Cocoa.h>

#import "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "exo/exo_browser/browser/ui/dialog/javascript_dialog_manager.h"

using namespace content;

// Helper object that receives the notification that the dialog/sheet is
// going away. Is responsible for cleaning itself up.
@interface JavaScriptDialogHelper : NSObject<NSAlertDelegate> {
 @private
  base::scoped_nsobject<NSAlert> alert_;
  NSTextField* textField_;  // WEAK; owned by alert_

  // Copies of the fields in JavaScriptDialog because they're private.
  exo_browser::ExoBrowserJavaScriptDialogManager* manager_;
  JavaScriptDialogManager::DialogClosedCallback callback_;
}

- (id)initHelperWithManager:
     (exo_browser::ExoBrowserJavaScriptDialogManager*)manager
                andCallback:
     (JavaScriptDialogManager::DialogClosedCallback)callback;
- (NSAlert*)alert;
- (NSTextField*)textField;
- (void)alertDidEnd:(NSAlert*)alert
         returnCode:(int)returnCode
        contextInfo:(void*)contextInfo;
- (void)cancel;

@end

@implementation JavaScriptDialogHelper

- (id)initHelperWithManager:
    (exo_browser::ExoBrowserJavaScriptDialogManager*)manager
                andCallback:
    (JavaScriptDialogManager::DialogClosedCallback)callback {
  if (self = [super init]) {
    manager_ = manager;
    callback_ = callback;
  }

  return self;
}

- (NSAlert*)alert {
  alert_.reset([[NSAlert alloc] init]);
  return alert_;
}

- (NSTextField*)textField {
  textField_ = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 22)];
  [[textField_ cell] setLineBreakMode:NSLineBreakByTruncatingTail];
  [alert_ setAccessoryView:textField_];
  [textField_ release];

  return textField_;
}

- (void)alertDidEnd:(NSAlert*)alert
         returnCode:(int)returnCode
        contextInfo:(void*)contextInfo {
  if (returnCode == NSRunStoppedResponse)
    return;

  bool success = returnCode == NSAlertFirstButtonReturn;
  string16 input;
  if (textField_)
    input = base::SysNSStringToUTF16([textField_ stringValue]);

  exo_browser::JavaScriptDialog* native_dialog =
      reinterpret_cast<exo_browser::JavaScriptDialog*>(contextInfo);
  callback_.Run(success, input);
  manager_->DialogClosed(native_dialog);
}

- (void)cancel {
  [NSApp endSheet:[alert_ window]];
  alert_.reset();
}

@end

namespace exo_browser {

JavaScriptDialog::JavaScriptDialog(
    ExoBrowserJavaScriptDialogManager* manager,
    gfx::NativeWindow parent_window,
    JavaScriptMessageType message_type,
    const string16& message_text,
    const string16& default_prompt_text,
    const JavaScriptDialogManager::DialogClosedCallback& callback)
    : manager_(manager),
      callback_(callback) 
{
  bool text_field = message_type == JAVASCRIPT_MESSAGE_TYPE_PROMPT;
  bool one_button = message_type == JAVASCRIPT_MESSAGE_TYPE_ALERT;

  helper_ =
      [[JavaScriptDialogHelper alloc] initHelperWithManager:manager
                                                andCallback:callback];

  // Show the modal dialog.
  NSAlert* alert = [helper_ alert];
  NSTextField* field = nil;
  if (text_field) {
    field = [helper_ textField];
    [field setStringValue:base::SysUTF16ToNSString(default_prompt_text)];
  }
  [alert setDelegate:helper_];
  [alert setInformativeText:base::SysUTF16ToNSString(message_text)];
  [alert setMessageText:@"Javascript alert"];
  [alert addButtonWithTitle:@"OK"];
  if (!one_button) {
    NSButton* other = [alert addButtonWithTitle:@"Cancel"];
    [other setKeyEquivalent:@"\e"];
  }

  [alert
      beginSheetModalForWindow:nil  // nil here makes it app-modal
                 modalDelegate:helper_
                didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
                   contextInfo:this];

  if ([alert accessoryView])
    [[alert window] makeFirstResponder:[alert accessoryView]];
}

JavaScriptDialog::~JavaScriptDialog() 
{
  [helper_ release];
}

void 
JavaScriptDialog::Cancel() 
{
  [helper_ cancel];
}

} // namespace exo_browser
