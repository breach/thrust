// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/dialog/download_manager_delegate.h"

#include <AppKit/NSSavePanel.h>
#include <Foundation/NSURL.h>

#include "base/bind.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "net/base/net_util.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"

using namespace content;

namespace exo_shell {

void ExoShellDownloadManagerDelegate::ChooseDownloadPath(
    uint32 download_id,
    const DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DownloadItem* item = download_manager_->GetDownload(download_id);
  if (!item || (item->GetState() != DownloadItem::IN_PROGRESS))
    return;

  base::FilePath result;
  std::string base_name = base::FilePath(suggested_path).BaseName().value();

  NSSavePanel *savePanel = [NSSavePanel savePanel];

  [savePanel setNameFieldStringValue:[NSString stringWithUTF8String:base_name.c_str()]];

  if ([savePanel runModal] == NSFileHandlingPanelOKButton) {
    char *filename = (char *)[[[savePanel URL] path] UTF8String];
    result = base::FilePath(filename);
  }

  callback.Run(result, DownloadItem::TARGET_DISPOSITION_PROMPT,
               DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, result);
}

} // namespace exo_shell
