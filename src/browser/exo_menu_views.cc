// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#include "src/browser/exo_menu.h"

#include "ui/gfx/screen.h"
#include "ui/views/controls/menu/menu_runner.h"

#include "src/browser/exo_shell.h"

using namespace content;

namespace exo_shell {

void 
ExoMenu::PlatformPopup(
    ExoShell* shell) 
{
  gfx::Point cursor = gfx::Screen::GetNativeScreen()->GetCursorScreenPoint();
  views::MenuRunner menu_runner(model());
  ignore_result(menu_runner.RunMenuAt(
      shell->window_.get(),
      NULL,
      gfx::Rect(cursor, gfx::Size()),
      views::MENU_ANCHOR_TOPLEFT,
      ui::MENU_SOURCE_MOUSE,
      views::MenuRunner::HAS_MNEMONICS | views::MenuRunner::CONTEXT_MENU));
}

} // namespace exo_shell
