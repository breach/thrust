// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/visitedlink/renderer/visitedlink_slave.h"

#include "base/logging.h"
#include "base/memory/shared_memory.h"
#include "components/visitedlink/common/visitedlink_messages.h"
#include "third_party/WebKit/public/web/WebView.h"

using blink::WebView;

namespace visitedlink {

VisitedLinkSlave::VisitedLinkSlave() : shared_memory_(NULL) {}

VisitedLinkSlave::~VisitedLinkSlave() {
  FreeTable();
}

bool VisitedLinkSlave::OnControlMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(VisitedLinkSlave, message)
    IPC_MESSAGE_HANDLER(ChromeViewMsg_VisitedLink_NewTable,
                        OnUpdateVisitedLinks)
    IPC_MESSAGE_HANDLER(ChromeViewMsg_VisitedLink_Add, OnAddVisitedLinks)
    IPC_MESSAGE_HANDLER(ChromeViewMsg_VisitedLink_Reset, OnResetVisitedLinks)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

// This function's job is to initialize the table with the given
// shared memory handle. This memory is mapped into the process.
void VisitedLinkSlave::OnUpdateVisitedLinks(base::SharedMemoryHandle table) {
  DCHECK(base::SharedMemory::IsHandleValid(table)) << "Bad table handle";
  // since this function may be called again to change the table, we may need
  // to free old objects
  FreeTable();
  DCHECK(shared_memory_ == NULL && hash_table_ == NULL);

  // create the shared memory object
  shared_memory_ = new base::SharedMemory(table, true);
  if (!shared_memory_)
    return;

  // map the header into our process so we can see how long the rest is,
  // and set the salt
  if (!shared_memory_->Map(sizeof(SharedHeader)))
    return;
  SharedHeader* header =
    static_cast<SharedHeader*>(shared_memory_->memory());
  DCHECK(header);
  int32 table_len = header->length;
  memcpy(salt_, header->salt, sizeof(salt_));
  shared_memory_->Unmap();

  // now do the whole table because we know the length
  if (!shared_memory_->Map(sizeof(SharedHeader) +
                          table_len * sizeof(Fingerprint))) {
    shared_memory_->Close();
    return;
  }

  // commit the data
  DCHECK(shared_memory_->memory());
  hash_table_ = reinterpret_cast<Fingerprint*>(
      static_cast<char*>(shared_memory_->memory()) + sizeof(SharedHeader));
  table_length_ = table_len;
}

void VisitedLinkSlave::OnAddVisitedLinks(
    const VisitedLinkSlave::Fingerprints& fingerprints) {
  for (size_t i = 0; i < fingerprints.size(); ++i)
    WebView::updateVisitedLinkState(fingerprints[i]);
}

void VisitedLinkSlave::OnResetVisitedLinks() {
  WebView::resetVisitedLinkState();
}

void VisitedLinkSlave::FreeTable() {
  if (shared_memory_) {
    delete shared_memory_;
    shared_memory_ = NULL;
  }
  hash_table_ = NULL;
  table_length_ = 0;
}

}  // namespace visitedlink
