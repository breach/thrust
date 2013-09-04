// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_DOWNLOAD_MANAGER_DELEGATE_H_
#define EXO_BROWSER_BROWSER_DOWNLOAD_MANAGER_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/download_manager_delegate.h"

namespace content {
class DownloadManager;
}

namespace exo_browser {

class ExoBrowserDownloadManagerDelegate
    : public content::DownloadManagerDelegate,
      public base::RefCountedThreadSafe<ExoBrowserDownloadManagerDelegate> {
 public:
  ExoBrowserDownloadManagerDelegate();

  void SetDownloadManager(content::DownloadManager* manager);

  virtual void Shutdown() OVERRIDE;
  virtual bool DetermineDownloadTarget(
      content::DownloadItem* download,
      const content::DownloadTargetCallback& callback) OVERRIDE;
  virtual bool ShouldOpenDownload(
      content::DownloadItem* item,
      const content::DownloadOpenDelayedCallback& callback) OVERRIDE;
  virtual void GetNextId(const content::DownloadIdCallback& callback) OVERRIDE;

  // Inhibits prompting and sets the default download path.
  void SetDownloadBehaviorForTesting(
      const base::FilePath& default_download_path);

 protected:
  // To allow subclasses for testing.
  virtual ~ExoBrowserDownloadManagerDelegate();

 private:
  friend class base::RefCountedThreadSafe<ExoBrowserDownloadManagerDelegate>;


  void GenerateFilename(uint32 download_id,
                        const content::DownloadTargetCallback& callback,
                        const base::FilePath& generated_name,
                        const base::FilePath& suggested_directory);
  void OnDownloadPathGenerated(uint32 download_id,
                               const content::DownloadTargetCallback& callback,
                               const base::FilePath& suggested_path);
  void ChooseDownloadPath(uint32 download_id,
                          const content::DownloadTargetCallback& callback,
                          const base::FilePath& suggested_path);

  content::DownloadManager* download_manager_;
  base::FilePath default_download_path_;
  bool suppress_prompting_;

  DISALLOW_COPY_AND_ASSIGN(ExoBrowserDownloadManagerDelegate);
};

} // namespace exo_browser

#endif // EXO_BROWSER_BROWSER_DOWNLOAD_MANAGER_DELEGATE_H_
