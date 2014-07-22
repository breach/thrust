// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/visitedlink/browser/visitedlink_master.h"

#if defined(OS_WIN)
#include <windows.h>
#include <io.h>
#include <shlobj.h>
#endif  // defined(OS_WIN)
#include <stdio.h>

#include <algorithm>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/containers/stack_container.h"
#include "base/file_util.h"
#include "base/files/scoped_file.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/rand_util.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_restrictions.h"
#include "components/visitedlink/browser/visitedlink_delegate.h"
#include "components/visitedlink/browser/visitedlink_event_listener.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "url/gurl.h"

using content::BrowserThread;

namespace visitedlink {

const int32 VisitedLinkMaster::kFileHeaderSignatureOffset = 0;
const int32 VisitedLinkMaster::kFileHeaderVersionOffset = 4;
const int32 VisitedLinkMaster::kFileHeaderLengthOffset = 8;
const int32 VisitedLinkMaster::kFileHeaderUsedOffset = 12;
const int32 VisitedLinkMaster::kFileHeaderSaltOffset = 16;

const int32 VisitedLinkMaster::kFileCurrentVersion = 3;

// the signature at the beginning of the URL table = "VLnk" (visited links)
const int32 VisitedLinkMaster::kFileSignature = 0x6b6e4c56;
const size_t VisitedLinkMaster::kFileHeaderSize =
    kFileHeaderSaltOffset + LINK_SALT_LENGTH;

// This value should also be the same as the smallest size in the lookup
// table in NewTableSizeForCount (prime number).
const unsigned VisitedLinkMaster::kDefaultTableSize = 16381;

const size_t VisitedLinkMaster::kBigDeleteThreshold = 64;

namespace {

// Fills the given salt structure with some quasi-random values
// It is not necessary to generate a cryptographically strong random string,
// only that it be reasonably different for different users.
void GenerateSalt(uint8 salt[LINK_SALT_LENGTH]) {
  DCHECK_EQ(LINK_SALT_LENGTH, 8) << "This code assumes the length of the salt";
  uint64 randval = base::RandUint64();
  memcpy(salt, &randval, 8);
}

// Opens file on a background thread to not block UI thread.
void AsyncOpen(FILE** file, const base::FilePath& filename) {
  *file = base::OpenFile(filename, "wb+");
  DLOG_IF(ERROR, !(*file)) << "Failed to open file " << filename.value();
}

// Returns true if the write was complete.
static bool WriteToFile(FILE* file,
                        off_t offset,
                        const void* data,
                        size_t data_len) {
  if (fseek(file, offset, SEEK_SET) != 0)
    return false;  // Don't write to an invalid part of the file.

  size_t num_written = fwrite(data, 1, data_len, file);

  // The write may not make it to the kernel (stdlib may buffer the write)
  // until the next fseek/fclose call.  If we crash, it's easy for our used
  // item count to be out of sync with the number of hashes we write.
  // Protect against this by calling fflush.
  int ret = fflush(file);
  DCHECK_EQ(0, ret);
  return num_written == data_len;
}

// This task executes on a background thread and executes a write. This
// prevents us from blocking the UI thread doing I/O. Double pointer to FILE
// is used because file may still not be opened by the time of scheduling
// the task for execution.
void AsyncWrite(FILE** file, int32 offset, const std::string& data) {
  if (*file)
    WriteToFile(*file, offset, data.data(), data.size());
}

// Truncates the file to the current position asynchronously on a background
// thread. Double pointer to FILE is used because file may still not be opened
// by the time of scheduling the task for execution.
void AsyncTruncate(FILE** file) {
  if (*file)
    base::IgnoreResult(base::TruncateFile(*file));
}

// Closes the file on a background thread and releases memory used for storage
// of FILE* value. Double pointer to FILE is used because file may still not
// be opened by the time of scheduling the task for execution.
void AsyncClose(FILE** file) {
  if (*file)
    base::IgnoreResult(fclose(*file));
  free(file);
}

}  // namespace

// TableBuilder ---------------------------------------------------------------

// How rebuilding from history works
// ---------------------------------
//
// We mark that we're rebuilding from history by setting the table_builder_
// member in VisitedLinkMaster to the TableBuilder we create. This builder
// will be called on the history thread by the history system for every URL
// in the database.
//
// The builder will store the fingerprints for those URLs, and then marshalls
// back to the main thread where the VisitedLinkMaster will be notified. The
// master then replaces its table with a new table containing the computed
// fingerprints.
//
// The builder must remain active while the history system is using it.
// Sometimes, the master will be deleted before the rebuild is complete, in
// which case it notifies the builder via DisownMaster(). The builder will
// delete itself once rebuilding is complete, and not execute any callback.
class VisitedLinkMaster::TableBuilder
    : public VisitedLinkDelegate::URLEnumerator {
 public:
  TableBuilder(VisitedLinkMaster* master,
               const uint8 salt[LINK_SALT_LENGTH]);

  // Called on the main thread when the master is being destroyed. This will
  // prevent a crash when the query completes and the master is no longer
  // around. We can not actually do anything but mark this fact, since the
  // table will be being rebuilt simultaneously on the other thread.
  void DisownMaster();

  // VisitedLinkDelegate::URLEnumerator
  virtual void OnURL(const GURL& url) OVERRIDE;
  virtual void OnComplete(bool succeed) OVERRIDE;

 private:
  virtual ~TableBuilder() {}

  // OnComplete mashals to this function on the main thread to do the
  // notification.
  void OnCompleteMainThread();

  // Owner of this object. MAY ONLY BE ACCESSED ON THE MAIN THREAD!
  VisitedLinkMaster* master_;

  // Indicates whether the operation has failed or not.
  bool success_;

  // Salt for this new table.
  uint8 salt_[LINK_SALT_LENGTH];

  // Stores the fingerprints we computed on the background thread.
  VisitedLinkCommon::Fingerprints fingerprints_;

  DISALLOW_COPY_AND_ASSIGN(TableBuilder);
};

// VisitedLinkMaster ----------------------------------------------------------

VisitedLinkMaster::VisitedLinkMaster(content::BrowserContext* browser_context,
                                     VisitedLinkDelegate* delegate,
                                     bool persist_to_disk)
    : browser_context_(browser_context),
      delegate_(delegate),
      listener_(new VisitedLinkEventListener(this, browser_context)),
      persist_to_disk_(persist_to_disk) {
  InitMembers();
}

VisitedLinkMaster::VisitedLinkMaster(Listener* listener,
                                     VisitedLinkDelegate* delegate,
                                     bool persist_to_disk,
                                     bool suppress_rebuild,
                                     const base::FilePath& filename,
                                     int32 default_table_size)
    : browser_context_(NULL),
      delegate_(delegate),
      persist_to_disk_(persist_to_disk) {
  listener_.reset(listener);
  DCHECK(listener_.get());
  InitMembers();

  database_name_override_ = filename;
  table_size_override_ = default_table_size;
  suppress_rebuild_ = suppress_rebuild;
}

VisitedLinkMaster::~VisitedLinkMaster() {
  if (table_builder_.get()) {
    // Prevent the table builder from calling us back now that we're being
    // destroyed. Note that we DON'T delete the object, since the history
    // system is still writing into it. When that is complete, the table
    // builder will destroy itself when it finds we are gone.
    table_builder_->DisownMaster();
  }
  FreeURLTable();
  // FreeURLTable() will schedule closing of the file and deletion of |file_|.
  // So nothing should be done here.
}

void VisitedLinkMaster::InitMembers() {
  file_ = NULL;
  shared_memory_ = NULL;
  shared_memory_serial_ = 0;
  used_items_ = 0;
  table_size_override_ = 0;
  suppress_rebuild_ = false;
  sequence_token_ = BrowserThread::GetBlockingPool()->GetSequenceToken();

#ifndef NDEBUG
  posted_asynchronous_operation_ = false;
#endif
}

bool VisitedLinkMaster::Init() {
  // We probably shouldn't be loading this from the UI thread,
  // but it does need to happen early on in startup.
  //   http://code.google.com/p/chromium/issues/detail?id=24163
  base::ThreadRestrictions::ScopedAllowIO allow_io;

  if (persist_to_disk_) {
    if (InitFromFile())
      return true;
  }
  return InitFromScratch(suppress_rebuild_);
}

VisitedLinkMaster::Hash VisitedLinkMaster::TryToAddURL(const GURL& url) {
  // Extra check that we are not incognito. This should not happen.
  // TODO(boliu): Move this check to HistoryService when IsOffTheRecord is
  // removed from BrowserContext.
  if (browser_context_ && browser_context_->IsOffTheRecord()) {
    NOTREACHED();
    return null_hash_;
  }

  if (!url.is_valid())
    return null_hash_;  // Don't add invalid URLs.

  Fingerprint fingerprint = ComputeURLFingerprint(url.spec().data(),
                                                  url.spec().size(),
                                                  salt_);
  if (table_builder_.get()) {
    // If we have a pending delete for this fingerprint, cancel it.
    std::set<Fingerprint>::iterator found =
        deleted_since_rebuild_.find(fingerprint);
    if (found != deleted_since_rebuild_.end())
        deleted_since_rebuild_.erase(found);

    // A rebuild is in progress, save this addition in the temporary list so
    // it can be added once rebuild is complete.
    added_since_rebuild_.insert(fingerprint);
  }

  // If the table is "full", we don't add URLs and just drop them on the floor.
  // This can happen if we get thousands of new URLs and something causes
  // the table resizing to fail. This check prevents a hang in that case. Note
  // that this is *not* the resize limit, this is just a sanity check.
  if (used_items_ / 8 > table_length_ / 10)
    return null_hash_;  // Table is more than 80% full.

  return AddFingerprint(fingerprint, true);
}

void VisitedLinkMaster::PostIOTask(const tracked_objects::Location& from_here,
                                   const base::Closure& task) {
  DCHECK(persist_to_disk_);
  BrowserThread::GetBlockingPool()->PostSequencedWorkerTask(sequence_token_,
                                                            from_here, task);
}

void VisitedLinkMaster::AddURL(const GURL& url) {
  Hash index = TryToAddURL(url);
  if (!table_builder_.get() && index != null_hash_) {
    // Not rebuilding, so we want to keep the file on disk up-to-date.
    if (persist_to_disk_) {
      WriteUsedItemCountToFile();
      WriteHashRangeToFile(index, index);
    }
    ResizeTableIfNecessary();
  }
}

void VisitedLinkMaster::AddURLs(const std::vector<GURL>& url) {
  for (std::vector<GURL>::const_iterator i = url.begin();
       i != url.end(); ++i) {
    Hash index = TryToAddURL(*i);
    if (!table_builder_.get() && index != null_hash_)
      ResizeTableIfNecessary();
  }

  // Keeps the file on disk up-to-date.
  if (!table_builder_.get() && persist_to_disk_)
    WriteFullTable();
}

void VisitedLinkMaster::DeleteAllURLs() {
  // Any pending modifications are invalid.
  added_since_rebuild_.clear();
  deleted_since_rebuild_.clear();

  // Clear the hash table.
  used_items_ = 0;
  memset(hash_table_, 0, this->table_length_ * sizeof(Fingerprint));

  // Resize it if it is now too empty. Resize may write the new table out for
  // us, otherwise, schedule writing the new table to disk ourselves.
  if (!ResizeTableIfNecessary() && persist_to_disk_)
    WriteFullTable();

  listener_->Reset();
}

VisitedLinkDelegate* VisitedLinkMaster::GetDelegate() {
  return delegate_;
}

void VisitedLinkMaster::DeleteURLs(URLIterator* urls) {
  if (!urls->HasNextURL())
    return;

  listener_->Reset();

  if (table_builder_.get()) {
    // A rebuild is in progress, save this deletion in the temporary list so
    // it can be added once rebuild is complete.
    while (urls->HasNextURL()) {
      const GURL& url(urls->NextURL());
      if (!url.is_valid())
        continue;

      Fingerprint fingerprint =
          ComputeURLFingerprint(url.spec().data(), url.spec().size(), salt_);
      deleted_since_rebuild_.insert(fingerprint);

      // If the URL was just added and now we're deleting it, it may be in the
      // list of things added since the last rebuild. Delete it from that list.
      std::set<Fingerprint>::iterator found =
          added_since_rebuild_.find(fingerprint);
      if (found != added_since_rebuild_.end())
        added_since_rebuild_.erase(found);

      // Delete the URLs from the in-memory table, but don't bother writing
      // to disk since it will be replaced soon.
      DeleteFingerprint(fingerprint, false);
    }
    return;
  }

  // Compute the deleted URLs' fingerprints and delete them
  std::set<Fingerprint> deleted_fingerprints;
  while (urls->HasNextURL()) {
    const GURL& url(urls->NextURL());
    if (!url.is_valid())
      continue;
    deleted_fingerprints.insert(
        ComputeURLFingerprint(url.spec().data(), url.spec().size(), salt_));
  }
  DeleteFingerprintsFromCurrentTable(deleted_fingerprints);
}

// See VisitedLinkCommon::IsVisited which should be in sync with this algorithm
VisitedLinkMaster::Hash VisitedLinkMaster::AddFingerprint(
    Fingerprint fingerprint,
    bool send_notifications) {
  if (!hash_table_ || table_length_ == 0) {
    NOTREACHED();  // Not initialized.
    return null_hash_;
  }

  Hash cur_hash = HashFingerprint(fingerprint);
  Hash first_hash = cur_hash;
  while (true) {
    Fingerprint cur_fingerprint = FingerprintAt(cur_hash);
    if (cur_fingerprint == fingerprint)
      return null_hash_;  // This fingerprint is already in there, do nothing.

    if (cur_fingerprint == null_fingerprint_) {
      // End of probe sequence found, insert here.
      hash_table_[cur_hash] = fingerprint;
      used_items_++;
      // If allowed, notify listener that a new visited link was added.
      if (send_notifications)
        listener_->Add(fingerprint);
      return cur_hash;
    }

    // Advance in the probe sequence.
    cur_hash = IncrementHash(cur_hash);
    if (cur_hash == first_hash) {
      // This means that we've wrapped around and are about to go into an
      // infinite loop. Something was wrong with the hashtable resizing
      // logic, so stop here.
      NOTREACHED();
      return null_hash_;
    }
  }
}

void VisitedLinkMaster::DeleteFingerprintsFromCurrentTable(
    const std::set<Fingerprint>& fingerprints) {
  bool bulk_write = (fingerprints.size() > kBigDeleteThreshold);

  // Delete the URLs from the table.
  for (std::set<Fingerprint>::const_iterator i = fingerprints.begin();
       i != fingerprints.end(); ++i)
    DeleteFingerprint(*i, !bulk_write);

  // These deleted fingerprints may make us shrink the table.
  if (ResizeTableIfNecessary())
    return;  // The resize function wrote the new table to disk for us.

  // Nobody wrote this out for us, write the full file to disk.
  if (bulk_write && persist_to_disk_)
    WriteFullTable();
}

bool VisitedLinkMaster::DeleteFingerprint(Fingerprint fingerprint,
                                          bool update_file) {
  if (!hash_table_ || table_length_ == 0) {
    NOTREACHED();  // Not initialized.
    return false;
  }
  if (!IsVisited(fingerprint))
    return false;  // Not in the database to delete.

  // First update the header used count.
  used_items_--;
  if (update_file && persist_to_disk_)
    WriteUsedItemCountToFile();

  Hash deleted_hash = HashFingerprint(fingerprint);

  // Find the range of "stuff" in the hash table that is adjacent to this
  // fingerprint. These are things that could be affected by the change in
  // the hash table. Since we use linear probing, anything after the deleted
  // item up until an empty item could be affected.
  Hash end_range = deleted_hash;
  while (true) {
    Hash next_hash = IncrementHash(end_range);
    if (next_hash == deleted_hash)
      break;  // We wrapped around and the whole table is full.
    if (!hash_table_[next_hash])
      break;  // Found the last spot.
    end_range = next_hash;
  }

  // We could get all fancy and move the affected fingerprints around, but
  // instead we just remove them all and re-add them (minus our deleted one).
  // This will mean there's a small window of time where the affected links
  // won't be marked visited.
  base::StackVector<Fingerprint, 32> shuffled_fingerprints;
  Hash stop_loop = IncrementHash(end_range);  // The end range is inclusive.
  for (Hash i = deleted_hash; i != stop_loop; i = IncrementHash(i)) {
    if (hash_table_[i] != fingerprint) {
      // Don't save the one we're deleting!
      shuffled_fingerprints->push_back(hash_table_[i]);

      // This will balance the increment of this value in AddFingerprint below
      // so there is no net change.
      used_items_--;
    }
    hash_table_[i] = null_fingerprint_;
  }

  if (!shuffled_fingerprints->empty()) {
    // Need to add the new items back.
    for (size_t i = 0; i < shuffled_fingerprints->size(); i++)
      AddFingerprint(shuffled_fingerprints[i], false);
  }

  // Write the affected range to disk [deleted_hash, end_range].
  if (update_file && persist_to_disk_)
    WriteHashRangeToFile(deleted_hash, end_range);

  return true;
}

void VisitedLinkMaster::WriteFullTable() {
  // This function can get called when the file is open, for example, when we
  // resize the table. We must handle this case and not try to reopen the file,
  // since there may be write operations pending on the file I/O thread.
  //
  // Note that once we start writing, we do not delete on error. This means
  // there can be a partial file, but the short file will be detected next time
  // we start, and will be replaced.
  //
  // This might possibly get corrupted if we crash in the middle of writing.
  // We should pick up the most common types of these failures when we notice
  // that the file size is different when we load it back in, and then we will
  // regenerate the table.
  DCHECK(persist_to_disk_);

  if (!file_) {
    file_ = static_cast<FILE**>(calloc(1, sizeof(*file_)));
    base::FilePath filename;
    GetDatabaseFileName(&filename);
    PostIOTask(FROM_HERE, base::Bind(&AsyncOpen, file_, filename));
  }

  // Write the new header.
  int32 header[4];
  header[0] = kFileSignature;
  header[1] = kFileCurrentVersion;
  header[2] = table_length_;
  header[3] = used_items_;
  WriteToFile(file_, 0, header, sizeof(header));
  WriteToFile(file_, sizeof(header), salt_, LINK_SALT_LENGTH);

  // Write the hash data.
  WriteToFile(file_, kFileHeaderSize,
              hash_table_, table_length_ * sizeof(Fingerprint));

  // The hash table may have shrunk, so make sure this is the end.
  PostIOTask(FROM_HERE, base::Bind(&AsyncTruncate, file_));
}

bool VisitedLinkMaster::InitFromFile() {
  DCHECK(file_ == NULL);
  DCHECK(persist_to_disk_);

  base::FilePath filename;
  GetDatabaseFileName(&filename);
  base::ScopedFILE file_closer(base::OpenFile(filename, "rb+"));
  if (!file_closer.get())
    return false;

  int32 num_entries, used_count;
  if (!ReadFileHeader(file_closer.get(), &num_entries, &used_count, salt_))
    return false;  // Header isn't valid.

  // Allocate and read the table.
  if (!CreateURLTable(num_entries, false))
    return false;
  if (!ReadFromFile(file_closer.get(), kFileHeaderSize,
                    hash_table_, num_entries * sizeof(Fingerprint))) {
    FreeURLTable();
    return false;
  }
  used_items_ = used_count;

#ifndef NDEBUG
  DebugValidate();
#endif

  file_ = static_cast<FILE**>(malloc(sizeof(*file_)));
  *file_ = file_closer.release();
  return true;
}

bool VisitedLinkMaster::InitFromScratch(bool suppress_rebuild) {
  int32 table_size = kDefaultTableSize;
  if (table_size_override_)
    table_size = table_size_override_;

  // The salt must be generated before the table so that it can be copied to
  // the shared memory.
  GenerateSalt(salt_);
  if (!CreateURLTable(table_size, true))
    return false;

#ifndef NDEBUG
  DebugValidate();
#endif

  if (suppress_rebuild && persist_to_disk_) {
    // When we disallow rebuilds (normally just unit tests), just use the
    // current empty table.
    WriteFullTable();
    return true;
  }

  // This will build the table from history. On the first run, history will
  // be empty, so this will be correct. This will also write the new table
  // to disk. We don't want to save explicitly here, since the rebuild may
  // not complete, leaving us with an empty but valid visited link database.
  // In the future, we won't know we need to try rebuilding again.
  return RebuildTableFromDelegate();
}

bool VisitedLinkMaster::ReadFileHeader(FILE* file,
                                       int32* num_entries,
                                       int32* used_count,
                                       uint8 salt[LINK_SALT_LENGTH]) {
  DCHECK(persist_to_disk_);

  // Get file size.
  // Note that there is no need to seek back to the original location in the
  // file since ReadFromFile() [which is the next call accessing the file]
  // seeks before reading.
  if (fseek(file, 0, SEEK_END) == -1)
    return false;
  size_t file_size = ftell(file);

  if (file_size <= kFileHeaderSize)
    return false;

  uint8 header[kFileHeaderSize];
  if (!ReadFromFile(file, 0, &header, kFileHeaderSize))
    return false;

  // Verify the signature.
  int32 signature;
  memcpy(&signature, &header[kFileHeaderSignatureOffset], sizeof(signature));
  if (signature != kFileSignature)
    return false;

  // Verify the version is up-to-date. As with other read errors, a version
  // mistmatch will trigger a rebuild of the database from history, which will
  // have the effect of migrating the database.
  int32 version;
  memcpy(&version, &header[kFileHeaderVersionOffset], sizeof(version));
  if (version != kFileCurrentVersion)
    return false;  // Bad version.

  // Read the table size and make sure it matches the file size.
  memcpy(num_entries, &header[kFileHeaderLengthOffset], sizeof(*num_entries));
  if (*num_entries * sizeof(Fingerprint) + kFileHeaderSize != file_size)
    return false;  // Bad size.

  // Read the used item count.
  memcpy(used_count, &header[kFileHeaderUsedOffset], sizeof(*used_count));
  if (*used_count > *num_entries)
    return false;  // Bad used item count;

  // Read the salt.
  memcpy(salt, &header[kFileHeaderSaltOffset], LINK_SALT_LENGTH);

  // This file looks OK from the header's perspective.
  return true;
}

bool VisitedLinkMaster::GetDatabaseFileName(base::FilePath* filename) {
  if (!database_name_override_.empty()) {
    // use this filename, the directory must exist
    *filename = database_name_override_;
    return true;
  }

  if (!browser_context_ || browser_context_->GetPath().empty())
    return false;

  base::FilePath profile_dir = browser_context_->GetPath();
  *filename = profile_dir.Append(FILE_PATH_LITERAL("Visited Links"));
  return true;
}

// Initializes the shared memory structure. The salt should already be filled
// in so that it can be written to the shared memory
bool VisitedLinkMaster::CreateURLTable(int32 num_entries, bool init_to_empty) {
  // The table is the size of the table followed by the entries.
  uint32 alloc_size = num_entries * sizeof(Fingerprint) + sizeof(SharedHeader);

  // Create the shared memory object.
  shared_memory_ = new base::SharedMemory();
  if (!shared_memory_)
    return false;

  if (!shared_memory_->CreateAndMapAnonymous(alloc_size)) {
    delete shared_memory_;
    shared_memory_ = NULL;
    return false;
  }

  if (init_to_empty) {
    memset(shared_memory_->memory(), 0, alloc_size);
    used_items_ = 0;
  }
  table_length_ = num_entries;

  // Save the header for other processes to read.
  SharedHeader* header = static_cast<SharedHeader*>(shared_memory_->memory());
  header->length = table_length_;
  memcpy(header->salt, salt_, LINK_SALT_LENGTH);

  // Our table pointer is just the data immediately following the size.
  hash_table_ = reinterpret_cast<Fingerprint*>(
      static_cast<char*>(shared_memory_->memory()) + sizeof(SharedHeader));

  return true;
}

bool VisitedLinkMaster::BeginReplaceURLTable(int32 num_entries) {
  base::SharedMemory *old_shared_memory = shared_memory_;
  Fingerprint* old_hash_table = hash_table_;
  int32 old_table_length = table_length_;
  if (!CreateURLTable(num_entries, true)) {
    // Try to put back the old state.
    shared_memory_ = old_shared_memory;
    hash_table_ = old_hash_table;
    table_length_ = old_table_length;
    return false;
  }

#ifndef NDEBUG
  DebugValidate();
#endif

  return true;
}

void VisitedLinkMaster::FreeURLTable() {
  if (shared_memory_) {
    delete shared_memory_;
    shared_memory_ = NULL;
  }
  if (!persist_to_disk_ || !file_)
    return;
  PostIOTask(FROM_HERE, base::Bind(&AsyncClose, file_));
  // AsyncClose() will close the file and free the memory pointed by |file_|.
  file_ = NULL;
}

bool VisitedLinkMaster::ResizeTableIfNecessary() {
  DCHECK(table_length_ > 0) << "Must have a table";

  // Load limits for good performance/space. We are pretty conservative about
  // keeping the table not very full. This is because we use linear probing
  // which increases the likelihood of clumps of entries which will reduce
  // performance.
  const float max_table_load = 0.5f;  // Grow when we're > this full.
  const float min_table_load = 0.2f;  // Shrink when we're < this full.

  float load = ComputeTableLoad();
  if (load < max_table_load &&
      (table_length_ <= static_cast<float>(kDefaultTableSize) ||
       load > min_table_load))
    return false;

  // Table needs to grow or shrink.
  int new_size = NewTableSizeForCount(used_items_);
  DCHECK(new_size > used_items_);
  DCHECK(load <= min_table_load || new_size > table_length_);
  ResizeTable(new_size);
  return true;
}

void VisitedLinkMaster::ResizeTable(int32 new_size) {
  DCHECK(shared_memory_ && shared_memory_->memory() && hash_table_);
  shared_memory_serial_++;

#ifndef NDEBUG
  DebugValidate();
#endif

  base::SharedMemory* old_shared_memory = shared_memory_;
  Fingerprint* old_hash_table = hash_table_;
  int32 old_table_length = table_length_;
  if (!BeginReplaceURLTable(new_size))
    return;

  // Now we have two tables, our local copy which is the old one, and the new
  // one loaded into this object where we need to copy the data.
  for (int32 i = 0; i < old_table_length; i++) {
    Fingerprint cur = old_hash_table[i];
    if (cur)
      AddFingerprint(cur, false);
  }

  // On error unmapping, just forget about it since we can't do anything
  // else to release it.
  delete old_shared_memory;

  // Send an update notification to all child processes so they read the new
  // table.
  listener_->NewTable(shared_memory_);

#ifndef NDEBUG
  DebugValidate();
#endif

  // The new table needs to be written to disk.
  if (persist_to_disk_)
    WriteFullTable();
}

uint32 VisitedLinkMaster::NewTableSizeForCount(int32 item_count) const {
  // These table sizes are selected to be the maximum prime number less than
  // a "convenient" multiple of 1K.
  static const int table_sizes[] = {
      16381,    // 16K  = 16384   <- don't shrink below this table size
                //                   (should be == default_table_size)
      32767,    // 32K  = 32768
      65521,    // 64K  = 65536
      130051,   // 128K = 131072
      262127,   // 256K = 262144
      524269,   // 512K = 524288
      1048549,  // 1M   = 1048576
      2097143,  // 2M   = 2097152
      4194301,  // 4M   = 4194304
      8388571,  // 8M   = 8388608
      16777199,  // 16M  = 16777216
      33554347};  // 32M  = 33554432

  // Try to leave the table 33% full.
  int desired = item_count * 3;

  // Find the closest prime.
  for (size_t i = 0; i < arraysize(table_sizes); i ++) {
    if (table_sizes[i] > desired)
      return table_sizes[i];
  }

  // Growing very big, just approximate a "good" number, not growing as much
  // as normal.
  return item_count * 2 - 1;
}

// See the TableBuilder definition in the header file for how this works.
bool VisitedLinkMaster::RebuildTableFromDelegate() {
  DCHECK(!table_builder_.get());

  // TODO(brettw) make sure we have reasonable salt!
  table_builder_ = new TableBuilder(this, salt_);
  delegate_->RebuildTable(table_builder_);
  return true;
}

// See the TableBuilder declaration above for how this works.
void VisitedLinkMaster::OnTableRebuildComplete(
    bool success,
    const std::vector<Fingerprint>& fingerprints) {
  if (success) {
    // Replace the old table with a new blank one.
    shared_memory_serial_++;

    // We are responsible for freeing it AFTER it has been replaced if
    // replacement succeeds.
    base::SharedMemory* old_shared_memory = shared_memory_;

    int new_table_size = NewTableSizeForCount(
        static_cast<int>(fingerprints.size() + added_since_rebuild_.size()));
    if (BeginReplaceURLTable(new_table_size)) {
      // Free the old table.
      delete old_shared_memory;

      // Add the stored fingerprints to the hash table.
      for (size_t i = 0; i < fingerprints.size(); i++)
        AddFingerprint(fingerprints[i], false);

      // Also add anything that was added while we were asynchronously
      // generating the new table.
      for (std::set<Fingerprint>::iterator i = added_since_rebuild_.begin();
           i != added_since_rebuild_.end(); ++i)
        AddFingerprint(*i, false);
      added_since_rebuild_.clear();

      // Now handle deletions.
      DeleteFingerprintsFromCurrentTable(deleted_since_rebuild_);
      deleted_since_rebuild_.clear();

      // Send an update notification to all child processes.
      listener_->NewTable(shared_memory_);

      if (persist_to_disk_)
        WriteFullTable();
    }
  }
  table_builder_ = NULL;  // Will release our reference to the builder.

  // Notify the unit test that the rebuild is complete (will be NULL in prod.)
  if (!rebuild_complete_task_.is_null()) {
    rebuild_complete_task_.Run();
    rebuild_complete_task_.Reset();
  }
}

void VisitedLinkMaster::WriteToFile(FILE** file,
                                    off_t offset,
                                    void* data,
                                    int32 data_size) {
  DCHECK(persist_to_disk_);
#ifndef NDEBUG
  posted_asynchronous_operation_ = true;
#endif
  PostIOTask(FROM_HERE,
      base::Bind(&AsyncWrite, file, offset,
                 std::string(static_cast<const char*>(data), data_size)));
}

void VisitedLinkMaster::WriteUsedItemCountToFile() {
  DCHECK(persist_to_disk_);
  if (!file_)
    return;  // See comment on the file_ variable for why this might happen.
  WriteToFile(file_, kFileHeaderUsedOffset, &used_items_, sizeof(used_items_));
}

void VisitedLinkMaster::WriteHashRangeToFile(Hash first_hash, Hash last_hash) {
  DCHECK(persist_to_disk_);

  if (!file_)
    return;  // See comment on the file_ variable for why this might happen.
  if (last_hash < first_hash) {
    // Handle wraparound at 0. This first write is first_hash->EOF
    WriteToFile(file_, first_hash * sizeof(Fingerprint) + kFileHeaderSize,
                &hash_table_[first_hash],
                (table_length_ - first_hash + 1) * sizeof(Fingerprint));

    // Now do 0->last_lash.
    WriteToFile(file_, kFileHeaderSize, hash_table_,
                (last_hash + 1) * sizeof(Fingerprint));
  } else {
    // Normal case, just write the range.
    WriteToFile(file_, first_hash * sizeof(Fingerprint) + kFileHeaderSize,
                &hash_table_[first_hash],
                (last_hash - first_hash + 1) * sizeof(Fingerprint));
  }
}

bool VisitedLinkMaster::ReadFromFile(FILE* file,
                                     off_t offset,
                                     void* data,
                                     size_t data_size) {
  DCHECK(persist_to_disk_);
#ifndef NDEBUG
  // Since this function is synchronous, we require that no asynchronous
  // operations could possibly be pending.
  DCHECK(!posted_asynchronous_operation_);
#endif

  if (fseek(file, offset, SEEK_SET) != 0)
    return false;

  size_t num_read = fread(data, 1, data_size, file);
  return num_read == data_size;
}

// VisitedLinkTableBuilder ----------------------------------------------------

VisitedLinkMaster::TableBuilder::TableBuilder(
    VisitedLinkMaster* master,
    const uint8 salt[LINK_SALT_LENGTH])
    : master_(master),
      success_(true) {
  fingerprints_.reserve(4096);
  memcpy(salt_, salt, LINK_SALT_LENGTH * sizeof(uint8));
}

// TODO(brettw): Do we want to try to cancel the request if this happens? It
// could delay shutdown if there are a lot of URLs.
void VisitedLinkMaster::TableBuilder::DisownMaster() {
  master_ = NULL;
}

void VisitedLinkMaster::TableBuilder::OnURL(const GURL& url) {
  if (!url.is_empty()) {
    fingerprints_.push_back(VisitedLinkMaster::ComputeURLFingerprint(
        url.spec().data(), url.spec().length(), salt_));
  }
}

void VisitedLinkMaster::TableBuilder::OnComplete(bool success) {
  success_ = success;
  DLOG_IF(WARNING, !success) << "Unable to rebuild visited links";

  // Marshal to the main thread to notify the VisitedLinkMaster that the
  // rebuild is complete.
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&TableBuilder::OnCompleteMainThread, this));
}

void VisitedLinkMaster::TableBuilder::OnCompleteMainThread() {
  if (master_)
    master_->OnTableRebuildComplete(success_, fingerprints_);
}

}  // namespace visitedlink
