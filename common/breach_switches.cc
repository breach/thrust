// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/common/breach_switches.h"

namespace switches {

// Makes Breach use the given path for its data directory.
const char kBreachDataPath[] = "data-path";
// Makes Breach use the given path for its data directory.
const char kBreachRawInit[] = "breach-raw-init";

// Allow access to external pages during layout tests.
const char kAllowExternalPages[] = "allow-external-pages";

// Enable accelerated 2D canvas.
const char kEnableAccelerated2DCanvas[] = "enable-accelerated-2d-canvas";

// Encode binary layout test results (images, audio) using base64.
const char kEncodeBinary[] = "encode-binary";

}  // namespace switches
