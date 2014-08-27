// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/app/paths_mac.h"

#include "base/mac/bundle_locations.h"
#include "base/mac/foundation_util.h"
#include "base/path_service.h"
#include "content/public/common/content_paths.h"

namespace {

base::FilePath GetContentsPath() {
  // Start out with the path to the running executable.
  base::FilePath path;
  PathService::Get(base::FILE_EXE, &path);

  // Up to Contents.
  if (base::mac::IsBackgroundOnlyProcess()) {
    // The running executable is the helper. Go up five steps:
    // Contents/Frameworks/Helper.app/Contents/MacOS/Helper
    // ^ to here                                     ^ from here
    path = path.DirName().DirName().DirName().DirName().DirName();
  } else {
    // One step up to MacOS, another to Contents.
    path = path.DirName().DirName();
  }
  DCHECK_EQ("Contents", path.BaseName().value());

  return path;
}

base::FilePath GetFrameworksPath() {
  return GetContentsPath().Append("Frameworks");
}

}  // namespace

void OverrideFrameworkBundlePath() {
  base::FilePath helper_path =
      GetFrameworksPath().Append("ExoBrowser Framework.framework");

  base::mac::SetOverrideFrameworkBundlePath(helper_path);
}

void OverrideChildProcessPath() {
  base::FilePath helper_path =
      GetFrameworksPath().Append("ExoBrowser Helper.app")
                                        .Append("Contents")
                                        .Append("MacOS")
                                        .Append("ExoBrowser Helper");

  PathService::Override(content::CHILD_PROCESS_EXE, helper_path);
}

base::FilePath GetResourcesPakFilePath() {
  NSString* pak_path =
      [base::mac::FrameworkBundle() pathForResource:@"exo_browser"
                                             ofType:@"pak"];

  return base::FilePath([pak_path fileSystemRepresentation]);
}

base::FilePath GetInfoPlistPath() {
  return GetContentsPath().Append("Info.plist");
}
