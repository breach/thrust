// Copyright (c) 2014 GitHub, Inc. All rights reserved.
// Copyright (c) 2014 Michael Hernandez.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef EXO_BROWSER_BROWSER_UI_VIEWS_MENU_LAYOUT_H_
#define EXO_BROWSER_BROWSER_UI_VIEWS_MENU_LAYOUT_H_

#include "ui/views/layout/fill_layout.h"

namespace exo_browser {

class MenuLayout : public views::FillLayout {
 public:
  explicit MenuLayout(int menu_height);
  virtual ~MenuLayout();

  // views::LayoutManager:
  virtual void Layout(views::View* host) OVERRIDE;
  virtual gfx::Size GetPreferredSize(views::View* host) OVERRIDE;
  virtual int GetPreferredHeightForWidth(views::View* host, int width) OVERRIDE;

 private:
  bool HasMenu(const views::View* host) const;

  int menu_height_;

  DISALLOW_COPY_AND_ASSIGN(MenuLayout);
};

}  // namespace exo_browser

#endif  // EXO_BROWSER_BROWSER_UI_VIEWS_MENU_LAYOUT_H_
