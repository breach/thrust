// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// Copyright (c) 2014 GitHub, Inc.
// See the LICENSE file.

#ifndef EXO_SHELL_BROWSER_UI_VIEWS_WIN_FRAME_VIEW_H_
#define EXO_SHELL_BROWSER_UI_VIEWS_WIN_FRAME_VIEW_H_

#include "src/browser/ui/views/frameless_view.h"

namespace exo_shell {

class WinFrameView : public FramelessView {
 public:
  WinFrameView();
  virtual ~WinFrameView();

  /****************************************************************************/
  /* NONCLIENTFRAMEVIEW */
  /****************************************************************************/
  virtual gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const OVERRIDE;
  virtual int NonClientHitTest(const gfx::Point& point) OVERRIDE;

  /****************************************************************************/
  /* VIEWS OVERRIDE */
  /****************************************************************************/
  virtual gfx::Size GetMinimumSize() const OVERRIDE;
  virtual gfx::Size GetMaximumSize() const OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;

 private:
  void ClientAreaSizeToWindowSize(gfx::Size* size) const;

  DISALLOW_COPY_AND_ASSIGN(WinFrameView);
};

}  // namespace exo_shell

#endif  // EXO_SHELL_BROWSER_UI_VIEWS_WIN_FRAME_VIEW_H_
