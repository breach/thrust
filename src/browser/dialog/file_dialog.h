// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2014 GitHub, Inc.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_DIALOG_FILE_DIALOG_H_
#define THRUST_SHELL_BROWSER_DIALOG_FILE_DIALOG_H_

#include <string>
#include <utility>
#include <vector>

#include "base/callback_forward.h"
#include "base/files/file_path.h"

namespace thrust_shell {
class ThrustWindow;
}

namespace file_dialog {

// <description, extensions>
typedef std::pair<std::string, std::vector<std::string> > Filter;
typedef std::vector<Filter> Filters;

enum FileDialogProperty {
  FILE_DIALOG_OPEN_FILE        = 1 << 0,
  FILE_DIALOG_OPEN_DIRECTORY   = 1 << 1,
  FILE_DIALOG_MULTI_SELECTIONS = 1 << 2,
  FILE_DIALOG_CREATE_DIRECTORY = 1 << 3,
};

typedef base::Callback<void(
    bool result, const std::vector<base::FilePath>& paths)> OpenDialogCallback;

typedef base::Callback<void(
    bool result, const base::FilePath& path)> SaveDialogCallback;

bool ShowOpenDialog(thrust_shell::ThrustWindow* parent_window,
                    const std::string& title,
                    const base::FilePath& default_path,
                    const Filters& filters,
                    int properties,
                    std::vector<base::FilePath>* paths);

void ShowOpenDialog(thrust_shell::ThrustWindow* parent_window,
                    const std::string& title,
                    const base::FilePath& default_path,
                    const Filters& filters,
                    int properties,
                    const OpenDialogCallback& callback);

bool ShowSaveDialog(thrust_shell::ThrustWindow* parent_window,
                    const std::string& title,
                    const base::FilePath& default_path,
                    const Filters& filters,
                    base::FilePath* path);

void ShowSaveDialog(thrust_shell::ThrustWindow* parent_window,
                    const std::string& title,
                    const base::FilePath& default_path,
                    const Filters& filters,
                    const SaveDialogCallback& callback);

} // namespace file_dialog

#endif // THRUST_SHELL_BROWSER_DIALOG_FILE_DIALOG_H_
