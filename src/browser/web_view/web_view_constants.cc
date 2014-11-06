// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/browser/web_view/web_view_constants.h"

namespace webview {

// Events types.
const char kDidAttach[] = "did-attach";
const char kZoomChanged[] = "zoom-changed";
const char kDestroyed[] = "destroyed";

// Parameters/properties on events.
const char kIsTopLevel[] = "isTopLevel";
const char kReason[] = "reason";
const char kUrl[] = "url";

// Initialization parameters.
const char kParameterApi[] = "api";
const char kParameterInstanceId[] = "instanceId";

// Other.
const int kInstanceIDNone = 0;
const char kStoragePartitionId[] = "storagePartitionId";

}  // namespace webview
