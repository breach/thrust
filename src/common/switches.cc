// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/common/switches.h"

namespace switches {

// Makes ExoBrowser use the given path for its data directory.
const char kExoBrowserDataPath[] = "data-path";
// Prevents the launch of the default 'app/' and runs the in "raw" mode
const char kExoBrowserRaw[] = "raw";
// Dumps the 'shell/' in the current working directory 
const char kExoBrowserDumpShell[] = "dump-shell";

}  // namespace switches
