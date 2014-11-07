// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2014 GitHub, Inc.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_UI_VIEWS_FRAMELESS_VIEW_H_
#define THRUST_SHELL_BROWSER_UI_VIEWS_FRAMELESS_VIEW_H_

#include "ui/views/window/non_client_view.h"

namespace views {
class Widget;
}

namespace thrust_shell {

class ThrustWindow;

class FramelessView : public views::NonClientFrameView {
 public:
  FramelessView();
  virtual ~FramelessView();

  virtual void Init(ThrustWindow* window, views::Widget* frame);

  // Returns whether the |point| is on frameless window's resizing border.
  int ResizingBorderHitTest(const gfx::Point& point);

 protected:
  /****************************************************************************/
  /* NONCLIENTFRAMEVIEWS IMPLEMENTATION */
  /****************************************************************************/
  virtual gfx::Rect GetBoundsForClientView() const OVERRIDE;
  virtual gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const OVERRIDE;
  virtual int NonClientHitTest(const gfx::Point& point) OVERRIDE;
  virtual void GetWindowMask(const gfx::Size& size,
                             gfx::Path* window_mask) OVERRIDE;
  virtual void ResetWindowControls() OVERRIDE;
  virtual void UpdateWindowIcon() OVERRIDE;
  virtual void UpdateWindowTitle() OVERRIDE;

  /****************************************************************************/
  /* VIEW OVERRIDE */
  /****************************************************************************/
  virtual gfx::Size GetPreferredSize() const OVERRIDE;
  virtual gfx::Size GetMinimumSize() const OVERRIDE;
  virtual gfx::Size GetMaximumSize() const OVERRIDE;
  virtual const char* GetClassName() const OVERRIDE;

  // Not owned.
  ThrustWindow*      window_;
  views::Widget*     frame_;

 private:
  DISALLOW_COPY_AND_ASSIGN(FramelessView);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_UI_VIEWS_FRAMELESS_VIEW_H_
