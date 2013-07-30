// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "breach/browser/breach_download_manager_delegate.h"

#if defined(TOOLKIT_GTK)
#include <gtk/gtk.h>
#endif

#if defined(OS_WIN)
#include <windows.h>
#include <commdlg.h>
#endif

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
#include "breach/common/breach_switches.h"
#include "net/base/net_util.h"

using namespace content;

namespace content {

BreachDownloadManagerDelegate::BreachDownloadManagerDelegate()
  : download_manager_(NULL),
    suppress_prompting_(false) 
{
  // Balanced in Shutdown();
  AddRef();
}

BreachDownloadManagerDelegate::~BreachDownloadManagerDelegate()
{
}


void 
BreachDownloadManagerDelegate::SetDownloadManager(
    DownloadManager* download_manager) 
{
  download_manager_ = download_manager;
}

void 
BreachDownloadManagerDelegate::Shutdown() 
{
  Release();
}

bool 
BreachDownloadManagerDelegate::DetermineDownloadTarget(
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
          &BreachDownloadManagerDelegate::GenerateFilename,
          this, download->GetId(), callback, generated_name,
          default_download_path_));
  return true;
}

bool 
BreachDownloadManagerDelegate::ShouldOpenDownload(
    DownloadItem* item,
    const DownloadOpenDelayedCallback& callback) 
{
  if (CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree) &&
      WebKitTestController::Get()->IsMainWindow(item->GetWebContents()) &&
      item->GetMimeType() == "text/html") {
    WebKitTestController::Get()->OpenURL(
        net::FilePathToFileURL(item->GetFullPath()));
  }
  return true;
}

void 
BreachDownloadManagerDelegate::GetNextId(
    const DownloadIdCallback& callback) 
{
  static uint32 next_id = DownloadItem::kInvalidId + 1;
  callback.Run(next_id++);
}

void 
BreachDownloadManagerDelegate::GenerateFilename(
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
          &BreachDownloadManagerDelegate::OnDownloadPathGenerated,
          this, download_id, callback, suggested_path));
}

void 
BreachDownloadManagerDelegate::OnDownloadPathGenerated(
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

void 
BreachDownloadManagerDelegate::ChooseDownloadPath(
    uint32 download_id,
    const DownloadTargetCallback& callback,
    const base::FilePath& suggested_path) 
{
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DownloadItem* item = download_manager_->GetDownload(download_id);
  if (!item || (item->GetState() != DownloadItem::IN_PROGRESS))
    return;

  base::FilePath result;
#if defined(OS_WIN) && !defined(USE_AURA)
  std::wstring file_part = base::FilePath(suggested_path).BaseName().value();
  wchar_t file_name[MAX_PATH];
  base::wcslcpy(file_name, file_part.c_str(), arraysize(file_name));
  OPENFILENAME save_as;
  ZeroMemory(&save_as, sizeof(save_as));
  save_as.lStructSize = sizeof(OPENFILENAME);
  save_as.hwndOwner = item->GetWebContents()->GetView()->GetNativeView();
  save_as.lpstrFile = file_name;
  save_as.nMaxFile = arraysize(file_name);

  std::wstring directory;
  if (!suggested_path.empty())
    directory = suggested_path.DirName().value();

  save_as.lpstrInitialDir = directory.c_str();
  save_as.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING |
                  OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;

  if (GetSaveFileName(&save_as))
    result = base::FilePath(std::wstring(save_as.lpstrFile));
#elif defined(TOOLKIT_GTK)
  GtkWidget *dialog;
  gfx::NativeWindow parent_window;
  std::string base_name = base::FilePath(suggested_path).BaseName().value();

  parent_window = item->GetWebContents()->GetView()->GetTopLevelNativeWindow();
  dialog = gtk_file_chooser_dialog_new("Save File",
                                       parent_window,
                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                       NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
                                                 TRUE);
  gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
                                    base_name.c_str());

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    result = base::FilePath(filename);
    g_free(filename);
  }
  gtk_widget_destroy(dialog);
#else
  NOTIMPLEMENTED();
#endif

  callback.Run(result, DownloadItem::TARGET_DISPOSITION_PROMPT,
               DOWNLOAD_DANGER_TYPE_NOT_DANGEROUS, result);
}

void 
BreachDownloadManagerDelegate::SetDownloadBehaviorForTesting(
    const base::FilePath& default_download_path) 
{
  default_download_path_ = default_download_path;
  suppress_prompting_ = true;
}

} // namespace breach

