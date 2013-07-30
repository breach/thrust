// Copyright (c) 2013 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#ifndef BREACH_BROWSER_BREACH_DOWNLOAD_MANAGER_DELEGATE_H_
#define BREACH_BROWSER_BREACH_DOWNLOAD_MANAGER_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "content/public/browser/download_manager_delegate.h"

namespace breach {

class content::DownloadManager;

class BreachDownloadManagerDelegate
    : public content::DownloadManagerDelegate,
      public base::RefCountedThreadSafe<BreachDownloadManagerDelegate> {
 public:
  BreachDownloadManagerDelegate();

  void SetDownloadManager(DownloadManager* manager);

  virtual void Shutdown() OVERRIDE;
  virtual bool DetermineDownloadTarget(
      DownloadItem* download,
      const DownloadTargetCallback& callback) OVERRIDE;
  virtual bool ShouldOpenDownload(
      DownloadItem* item,
      const DownloadOpenDelayedCallback& callback) OVERRIDE;
  virtual void GetNextId(const DownloadIdCallback& callback) OVERRIDE;

  // Inhibits prompting and sets the default download path.
  void SetDownloadBehaviorForTesting(
      const base::FilePath& default_download_path);

 protected:
  // To allow subclasses for testing.
  virtual ~BreachDownloadManagerDelegate();

 private:
  friend class base::RefCountedThreadSafe<BreachDownloadManagerDelegate>;


  void GenerateFilename(uint32 download_id,
                        const DownloadTargetCallback& callback,
                        const base::FilePath& generated_name,
                        const base::FilePath& suggested_directory);
  void OnDownloadPathGenerated(uint32 download_id,
                               const DownloadTargetCallback& callback,
                               const base::FilePath& suggested_path);
  void ChooseDownloadPath(uint32 download_id,
                          const DownloadTargetCallback& callback,
                          const base::FilePath& suggested_path);

  DownloadManager* download_manager_;
  base::FilePath default_download_path_;
  bool suppress_prompting_;

  DISALLOW_COPY_AND_ASSIGN(BreachDownloadManagerDelegate);
};

} // namespace content

#endif // BREACH_BROWSER_BREACH_DOWNLOAD_MANAGER_DELEGATE_H_
