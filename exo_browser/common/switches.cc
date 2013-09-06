// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo/exo_browser/common/switches.h"

namespace switches {

// Makes ExoBrowser use the given path for its data directory.
const char kExoBrowserDataPath[] = "data-path";
// Prevents the launch of Exo and runs the ExoBrowser in "raw" mode
const char kExoBrowserRaw[] = "raw";

// Allow access to external pages during layout tests.
const char kAllowExternalPages[] = "allow-external-pages";

// Enable accelerated 2D canvas.
const char kEnableAccelerated2DCanvas[] = "enable-accelerated-2d-canvas";

// Encode binary layout test results (images, audio) using base64.
const char kEncodeBinary[] = "encode-binary";

}  // namespace switches
