// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/dialog/download_manager_delegate.h"

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
#include "net/base/net_util.h"
#include "net/base/filename_util.h"

using namespace content;

namespace thrust_shell {

ThrustShellDownloadManagerDelegate::ThrustShellDownloadManagerDelegate()
  : download_manager_(NULL),
    suppress_prompting_(false),
    weak_ptr_factory_(this)
{
}

ThrustShellDownloadManagerDelegate::~ThrustShellDownloadManagerDelegate()
{
  if (download_manager_) {
    DCHECK_EQ(static_cast<DownloadManagerDelegate*>(this),
              download_manager_->GetDelegate());
    download_manager_->SetDelegate(NULL);
    download_manager_ = NULL;
  }
  LOG(INFO) << "ThrustShellDownloadManagerDelegate Destructor";
}


void 
ThrustShellDownloadManagerDelegate::SetDownloadManager(
    DownloadManager* download_manager) 
{
  download_manager_ = download_manager;
}

void 
ThrustShellDownloadManagerDelegate::Shutdown() 
{
  // Revoke any pending callbacks. download_manager_ et. al. are no longer safe
  // to access after this point.
  weak_ptr_factory_.InvalidateWeakPtrs();
  download_manager_ = NULL;
}

bool 
ThrustShellDownloadManagerDelegate::DetermineDownloadTarget(
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

  FilenameDeterminedCallback filename_determined_callback =
      base::Bind(&ThrustShellDownloadManagerDelegate::OnDownloadPathGenerated,
                 weak_ptr_factory_.GetWeakPtr(),
                 download->GetId(),
                 callback);

  BrowserThread::PostTask(
      BrowserThread::FILE,
      FROM_HERE,
      base::Bind(&ThrustShellDownloadManagerDelegate::GenerateFilename,
                 download->GetURL(),
                 download->GetContentDisposition(),
                 download->GetSuggestedFilename(),
                 download->GetMimeType(),
                 default_download_path_,
                 filename_determined_callback));
  return true;
}

bool 
ThrustShellDownloadManagerDelegate::ShouldOpenDownload(
    DownloadItem* item,
    const DownloadOpenDelayedCallback& callback) 
{
  return true;
}

void 
ThrustShellDownloadManagerDelegate::GetNextId(
    const DownloadIdCallback& callback) 
{
  static uint32 next_id = DownloadItem::kInvalidId + 1;
  callback.Run(next_id++);
}

void 
ThrustShellDownloadManagerDelegate::GenerateFilename(
    const GURL& url,
    const std::string& content_disposition,
    const std::string& suggested_filename,
    const std::string& mime_type,
    const base::FilePath& suggested_directory,
    const FilenameDeterminedCallback& callback)
{
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));

  base::FilePath generated_name = net::GenerateFileName(url,
                                                        content_disposition,
                                                        std::string(),
                                                        suggested_filename,
                                                        mime_type,
                                                        "download");
  if (!base::PathExists(suggested_directory))
    base::CreateDirectory(suggested_directory);

  base::FilePath suggested_path(suggested_directory.Append(generated_name));
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE, base::Bind(callback, suggested_path));
}

void 
ThrustShellDownloadManagerDelegate::OnDownloadPathGenerated(
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

} // namespace thrust_shell

