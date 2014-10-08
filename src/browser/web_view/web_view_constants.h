// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2013 The Chromium Authors.
// See the LICENSE file.

// Constants used for the WebView API.

#ifndef EXO_SHELL_BROWSER_WEBVIEW_WEBVIEW_CONSTANTS_H_
#define EXO_SHELL_BROWSER_WEBVIEW_WEBVIEW_CONSTANTS_H_

namespace webview {

// Parameters/properties on events.
extern const char kIsTopLevel[];
extern const char kReason[];
extern const char kUrl[];

// Initialization parameters.
extern const char kParameterApi[];
extern const char kParameterInstanceId[];

// Other.
extern const int kInstanceIDNone;
extern const char kStoragePartitionId[];

}  // namespace webview

#endif // EXO_SHELL_BROWSER_WEBVIEW_WEBVIEW_CONSTANTS_H_
