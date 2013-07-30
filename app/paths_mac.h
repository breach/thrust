// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_APP_PATHS_MAC_H_
#define BREACH_APP_PATHS_MAC_H_

namespace base {
class FilePath;
}

// Sets up base::mac::FrameworkBundle.
void OverrideFrameworkBundlePath();

// Sets up the CHILD_PROCESS_EXE path to properly point to the helper app.
void OverrideChildProcessPath();

// Gets the path to the content shell's pak file.
base::FilePath GetResourcesPakFilePath();

// Gets the path to content shell's Info.plist file.
base::FilePath GetInfoPlistPath();

#endif  // BREACH_APP_PATHS_MAC_H_
