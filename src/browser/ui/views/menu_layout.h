// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2014 GitHub, Inc. All rights reserved.
// See the LICENSE file.

#ifndef EXO_SHELL_BROWSER_UI_VIEWS_MENU_LAYOUT_H_
#define EXO_SHELL_BROWSER_UI_VIEWS_MENU_LAYOUT_H_

#include "ui/views/layout/fill_layout.h"

namespace exo_shell {

class MenuLayout : public views::FillLayout {
 public:
  explicit MenuLayout(int menu_height);
  virtual ~MenuLayout();

  /****************************************************************************/
  /* LAYOUTMANAGER IMPLEMENTATION */
  /****************************************************************************/
  virtual void Layout(views::View* host) OVERRIDE;
  virtual gfx::Size GetPreferredSize(const views::View* host) const OVERRIDE;
  virtual int GetPreferredHeightForWidth(
      const views::View* host, int width) const OVERRIDE;

 private:
  bool HasMenu(const views::View* host) const;

  int menu_height_;

  DISALLOW_COPY_AND_ASSIGN(MenuLayout);
};

}  // namespace exo_shell

#endif  // EXO_SHELL_BROWSER_UI_VIEWS_MENU_LAYOUT_H_
