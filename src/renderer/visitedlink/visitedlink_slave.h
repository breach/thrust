// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_VISITEDLINK_RENDERER_VISITEDLINK_SLAVE_H_
#define COMPONENTS_VISITEDLINK_RENDERER_VISITEDLINK_SLAVE_H_

#include "base/compiler_specific.h"
#include "base/memory/shared_memory.h"
#include "components/visitedlink/common/visitedlink_common.h"
#include "content/public/renderer/render_process_observer.h"

namespace visitedlink {

// Reads the link coloring database provided by the master. There can be any
// number of slaves reading the same database.
class VisitedLinkSlave : public VisitedLinkCommon,
                         public content::RenderProcessObserver {
 public:
  VisitedLinkSlave();
  virtual ~VisitedLinkSlave();

  // RenderProcessObserver implementation.
  virtual bool OnControlMessageReceived(const IPC::Message& message) OVERRIDE;

  // Message handlers.
  void OnUpdateVisitedLinks(base::SharedMemoryHandle table);
  void OnAddVisitedLinks(const VisitedLinkSlave::Fingerprints& fingerprints);
  void OnResetVisitedLinks();

 private:
  void FreeTable();

  // shared memory consists of a SharedHeader followed by the table
  base::SharedMemory* shared_memory_;

  DISALLOW_COPY_AND_ASSIGN(VisitedLinkSlave);
};

}  // namespace visitedlink

#endif  // COMPONENTS_VISITEDLINK_RENDERER_VISITEDLINK_SLAVE_H_
