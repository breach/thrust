// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#ifndef SRC_BROWSER_API_EXO_MENU_MAC_H_
#define SRC_BROWSER_API_EXO_MENU_MAC_H_

#include "src/browser/exo_menu.h"

#include <string>

#import "src/browser/ui/cocoa/exo_menu_controller.h"

namespace exo_shell {


class ExoMenuMac : public ExoMenu {
public:
  ExoMenuMac(ApiBinding* binding);

  virtual void Popup() OVERRIDE;

  base::scoped_nsobject<ExoMenuController> menu_controller_;


  friend class ExoMenu;

  static void SendActionToFirstResponder(const std::string& action);

  DISALLOW_COPY_AND_ASSIGN(ExoMenuMac);
  protected:
  private:
};


}  // namespace atom

#endif  // ATOM_BROWSER_API_EXO_API_MENU_MAC_H_
