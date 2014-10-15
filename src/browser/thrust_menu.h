// Copyright (c) 2014 Stanislas Polu. All rights reserved.
// See the LICENSE file.

#ifndef THRUST_SHELL_BROWSER_THRUST_MENU_H_
#define THRUST_SHELL_BROWSER_THRUST_MENU_H_

#include <string>
#include <map>

#include "base/memory/scoped_ptr.h"
#include "ui/base/models/simple_menu_model.h"

namespace thrust_shell {

class ThrustWindow;
class MenuController;
class ThrustMenuBinding;

class ThrustMenu : public ui::SimpleMenuModel::Delegate {
public:
  /****************************************************************************/
  /* PUBLIC API */
  /****************************************************************************/
  // ### ThrustMenu
  ThrustMenu(ThrustMenuBinding* binding);
  // ### ~ThrustMenu
  virtual ~ThrustMenu();

  void AddItem(int command_id, 
               const base::string16& label);
  void AddSeparator();
  void AddCheckItem(int command_id,
                    const base::string16& label);
  void AddRadioItem(int command_id,
                    const base::string16& label,
                    int group_id);
  void AddSubMenu(int command_id,
                  const base::string16& label,
                  ThrustMenu* menu);

  void Clear();

  void SetChecked(int command_id, bool checked);
  void SetEnabled(int command_id, bool enabled);
  void SetVisible(int command_id, bool visible);
  void SetAccelerator(int command_id, std::string accelerator);

  int GetIndexOfCommandId(int command_id);
  int GetItemCount() const;
  int GetCommandIdAt(int index) const;
  base::string16 GetLabelAt(int index) const;

  bool IsItemCheckedAt(int index) const;
  bool IsEnabledAt(int index) const;
  bool IsVisibleAt(int index) const;

  virtual void AttachToWindow(ThrustWindow* window);
  virtual void Popup(ThrustWindow* window) {
    return PlatformPopup(window);
  }

#if defined(OS_MACOSX)
  // Set the global menubar.
  static void SetApplicationMenu(ThrustMenu* menu);
  // Fake sending an action from the application menu.
  static void SendActionToFirstResponder(const std::string& action);
#endif

  ui::SimpleMenuModel* model() const { return model_.get(); }

 protected:
  /****************************************************************************/
  /* SIMPLEMENUMODEL DELEGATE IMPLEMENTATION */
  /****************************************************************************/
  virtual bool IsCommandIdChecked(int command_id) const OVERRIDE;
  virtual bool IsCommandIdEnabled(int command_id) const OVERRIDE;
  virtual bool IsCommandIdVisible(int command_id) const OVERRIDE;

  virtual bool GetAcceleratorForCommandId(
      int command_id,
      ui::Accelerator* accelerator) OVERRIDE;

  virtual void ExecuteCommand(int command_id, int event_flags) OVERRIDE;
  virtual void MenuWillShow(ui::SimpleMenuModel* source) OVERRIDE;
  virtual void MenuClosed(ui::SimpleMenuModel* source) OVERRIDE;

 private:
  /****************************************************************************/
  /* PLATFORM INTERFACE */
  /****************************************************************************/
  void PlatformPopup(ThrustWindow* window);

  ThrustMenuBinding*                        binding_;

  scoped_ptr<ui::SimpleMenuModel>           model_;
  ThrustMenu*                               parent_;

  std::map<int, bool>                       checked_;
  std::map<int, bool>                       enabled_;
  std::map<int, bool>                       visible_;
  std::map<int, std::string>                accelerator_;

  DISALLOW_COPY_AND_ASSIGN(ThrustMenu);
};

} // namespace thrust_shell

#endif // THRUST_SHELL_BROWSER_THRUST_MENU_H_
