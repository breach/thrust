// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "exo_browser/src/browser/dialog/download_manager_delegate.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "exo_browser/src/common/switches.h"
#include "net/base/net_util.h"

using namespace content;

namespace exo_browser {

ExoBrowserDownloadManagerDelegate::ExoBrowserDownloadManagerDelegate()
  : download_manager_(NULL),
    suppress_prompting_(false) 
{
  // Balanced in Shutdown();
  AddRef();
}

ExoBrowserDownloadManagerDelegate::~ExoBrowserDownloadManagerDelegate()
{
}


void 
ExoBrowserDownloadManagerDelegate::SetDownloadManager(
    DownloadManager* download_manager) 
{
  download_manager_ = download_manager;
}

void 
ExoBrowserDownloadManagerDelegate::Shutdown() 
{
  Release();
}

bool 
ExoBrowserDownloadManagerDelegate::DetermineDownloadTarget(
    DownloadItem* download,
    const DownloadTargetCallback& callback) 
{
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  // This assignment needs to be here because even at the call to
  // SetDownloadManager, the system is not fully initialized.
  if (default_download_path_.empty()) {
    default_download_path_ = download_manager_->GetBrowserContext()->GetPath().
        Append(FILE_PATH_LITERAL("Downloads"));
  }

  if (!download->GetForcedFilePath().empty()) {
    callback.Run(download->GetForcedFilePath(),
                 DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                 DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 download->GetForcedFilePath());
    return true;
  }

  base::FilePath generated_name = net::GenerateFileName(
      download->GetURL(),
      download->GetContentDisposition(),
      EmptyString(),
      download->GetSuggestedFilename(),
      download->GetMimeType(),
      "download");

  BrowserThread::PostTask(
      BrowserThread::FILE,
      FROM_HERE,
      base::Bind(
          &ExoBrowserDownloadManagerDelegate::GenerateFilename,
          this, download->GetId(), callback, generated_name,
          default_download_path_));
  return true;
}

bool 
ExoBrowserDownloadManagerDelegate::ShouldOpenDownload(
    DownloadItem* item,
    const DownloadOpenDelayedCallback& callback) 
{
  return true;
}

void 
ExoBrowserDownloadManagerDelegate::GetNextId(
    const DownloadIdCallback& callback) 
{
  static uint32 next_id = DownloadItem::kInvalidId + 1;
  callback.Run(next_id++);
}

void 
ExoBrowserDownloadManagerDelegate::GenerateFilename(
    uint32 download_id,
    const DownloadTargetCallback& callback,
    const base::FilePath& generated_name,
    const base::FilePath& suggested_directory) 
{
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  if (!base::PathExists(suggested_directory))
    file_util::CreateDirectory(suggested_directory);

  base::FilePath suggested_path(suggested_directory.Append(generated_name));
  BrowserThread::PostTask(
      BrowserThread::UI,
      FROM_HERE,
      base::Bind(
          &ExoBrowserDownloadManagerDelegate::OnDownloadPathGenerated,
          this, download_id, callback, suggested_path));
}

void 
ExoBrowserDownloadManagerDelegate::OnDownloadPathGenerated(
    uint32 download_id,
    const DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) 
{
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  if (suppress_prompting_) {
    // Testing exit.
    callback.Run(suggested_path, DownloadItem::TARGET_DISPOSITION_OVERWRITE,
                 DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS,
                 suggested_path.AddExtension(FILE_PATH_LITERAL(".crdownload")));
    return;
  }

  ChooseDownloadPath(download_id, callback, suggested_path);
}

} // namespace exo_browser

