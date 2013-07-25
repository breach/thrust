// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/common/breach_switches.h"

namespace switches {

// Allow access to external pages during layout tests.
const char kAllowExternalPages[] = "allow-external-pages";

// Makes Breach use the given path for its data directory.
const char kContentBreachDataPath[] = "data-path";

// Enable accelerated 2D canvas.
const char kEnableAccelerated2DCanvas[] = "enable-accelerated-2d-canvas";

// Alias for kEnableSoftwareCompositingGLAdapter.
const char kEnableSoftwareCompositing[] = "enable-software-compositing";

// Encode binary layout test results (images, audio) using base64.
const char kEncodeBinary[] = "encode-binary";

}  // namespace switches
