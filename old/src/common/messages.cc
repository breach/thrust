// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

// Get basic type definitions.
#define IPC_MESSAGE_IMPL
#include "exo_browser/src/common/messages.h"

// Generate constructors.
#include "ipc/struct_constructor_macros.h"
#include "exo_browser/src/common/messages.h"

// Generate destructors.
#include "ipc/struct_destructor_macros.h"
#include "exo_browser/src/common/messages.h"

// Generate param traits write methods.
#include "ipc/param_traits_write_macros.h"
namespace IPC {
#include "exo_browser/src/common/messages.h"
}  // namespace IPC

// Generate param traits read methods.
#include "ipc/param_traits_read_macros.h"
namespace IPC {
#include "exo_browser/src/common/messages.h"
}  // namespace IPC

// Generate param traits log methods.
#include "ipc/param_traits_log_macros.h"
namespace IPC {
#include "exo_browser/src/common/messages.h"
}  // namespace IPC
