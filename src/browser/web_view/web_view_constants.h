// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2013 The Chromium Authors.
// See the LICENSE file.

// Constants used for the WebView API.

#ifndef THRUST_SHELL_BROWSER_WEBVIEW_WEBVIEW_CONSTANTS_H_
#define THRUST_SHELL_BROWSER_WEBVIEW_WEBVIEW_CONSTANTS_H_

namespace webview {


// Events types.
extern const char kDidAttach[];
extern const char kZoomChanged[];
extern const char kDestroyed[];

/* TODO(spolu): Finish and cleanup */

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

#endif // THRUST_SHELL_BROWSER_WEBVIEW_WEBVIEW_CONSTANTS_H_
