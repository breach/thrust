// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

// Multiply-included file, no traditional include guard.
#include <string>
#include <vector>

#include "base/values.h"
#include "content/public/common/common_param_traits.h"
#include "content/public/common/page_state.h"
#include "ipc/ipc_message_macros.h"
#include "ui/gfx/ipc/gfx_param_traits.h"

#include "src/common/draggable_region.h"

/* The message starter should be declared in ipc/ipc_message_start.h. Since */
/* we don't want to patch Chromium, we just pretend to be Content Shell.    */

#define IPC_MESSAGE_START ShellMsgStart

IPC_STRUCT_TRAITS_BEGIN(thrust_shell::DraggableRegion)
  IPC_STRUCT_TRAITS_MEMBER(draggable)
  IPC_STRUCT_TRAITS_MEMBER(bounds)
IPC_STRUCT_TRAITS_END()

// Sent by the renderer when the draggable regions are updated.
IPC_MESSAGE_ROUTED1(ThrustViewHostMsg_UpdateDraggableRegions,
                    std::vector<thrust_shell::DraggableRegion> /* regions */)


// CreateWebViewGuest
IPC_SYNC_MESSAGE_ROUTED1_1(ThrustFrameHostMsg_CreateWebViewGuest,
                           base::DictionaryValue, /* params */
                           int /* guest_instance_id */)
// DestroyWebViewGuest
IPC_MESSAGE_ROUTED1(ThrustFrameHostMsg_DestroyWebViewGuest,
                    int /* guest_instance_id */)

// WebViewGuestLoadUrl
IPC_MESSAGE_ROUTED2(ThrustFrameHostMsg_WebViewGuestLoadUrl,
                    int, /* guest_instance_id */
                    std::string /* url */)

// WebViewGuestGo
IPC_MESSAGE_ROUTED2(ThrustFrameHostMsg_WebViewGuestGo,
                    int, /* guest_instance_id */
                    int /* relative_index */)

// WebViewGuestReload
IPC_MESSAGE_ROUTED2(ThrustFrameHostMsg_WebViewGuestReload,
                    int, /* guest_instance_id */
                    int /* relative_index */)

// WebViewGuestEmit
IPC_MESSAGE_ROUTED3(ThrustFrameMsg_WebViewGuestEmit,
                    int, /* guest_instance_id */
                    std::string, /* type */
                    base::DictionaryValue /* event */);
