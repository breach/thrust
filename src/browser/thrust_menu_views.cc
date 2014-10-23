// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/thrust_menu.h"

#include "ui/gfx/screen.h"
#include "ui/views/controls/menu/menu_runner.h"

#include "src/browser/thrust_window.h"

using namespace content;

namespace thrust_shell {

void 
ThrustMenu::PlatformPopup(
    ThrustWindow* window) 
{
  gfx::Point cursor = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
  views::MenuRunner menu_runner(model());
  ignore_result(menu_runner.RunMenuAt(
      window->window_.get(),
      NULL,
      gfx::Rect(cursor, gfx::Size()),
      views::MENU_ANCHOR_TOPLEFT,
      ui::MENU_SOURCE_MOUSE,
      views::MenuRunner::HAS_MNEMONICS | views::MenuRunner::CONTEXT_MENU));
}

void
ThrustMenu::PlatformCleanup()
{
}


} // namespace thrust_shell
