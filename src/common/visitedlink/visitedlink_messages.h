// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, no traditional include guard.
#include <vector>

#include "base/basictypes.h"
#include "base/memory/shared_memory.h"
#include "content/public/common/common_param_traits_macros.h"
#include "ipc/ipc_message_macros.h"

#define IPC_MESSAGE_START VisitedLinkMsgStart

// History system notification that the visited link database has been
// replaced. It has one SharedMemoryHandle argument consisting of the table
// handle. This handle is valid in the context of the renderer
IPC_MESSAGE_CONTROL1(ChromeViewMsg_VisitedLink_NewTable,
                     base::SharedMemoryHandle)

// History system notification that a link has been added and the link
// coloring state for the given hash must be re-calculated.
IPC_MESSAGE_CONTROL1(ChromeViewMsg_VisitedLink_Add, std::vector<uint64>)

// History system notification that one or more history items have been
// deleted, which at this point means that all link coloring state must be
// re-calculated.
IPC_MESSAGE_CONTROL0(ChromeViewMsg_VisitedLink_Reset)

