// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

#ifndef EXO_SHELL_BROWSER_EXO_MENU_H_
#define EXO_SHELL_BROWSER_EXO_MENU_H_

#include <vector>

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/string_piece.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"
#include "ui/gfx/point.h"
#include "ui/gfx/image/image.h"


//#include "atom/browser/api/atom_api_window.h"
#include "base/memory/scoped_ptr.h"
#include "ui/base/models/simple_menu_model.h"

#if defined(USE_AURA)
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"
#endif

namespace exo_shell {
// ### ExoMenu
//
class ExoMenu : public ui::SimpleMenuModel::Delegate {

public:

  /* PUBLIC INTERFACE */
  /****************************************************************************/
  // ### ~ExoMenu
  ExoMenu();
  ~ExoMenu();



#if defined(OS_MACOSX)
  // Set the global menubar.
  static void SetApplicationMenu(ExoMenu* base_menu);

  // Fake sending an action from the application menu.
  static void SendActionToFirstResponder(const std::string& action);
#endif

  // ui::SimpleMenuModel* model() const { return model_.get(); }


 //  // ui::SimpleMenuModel::Delegate implementations:
   virtual bool IsCommandIdChecked(int command_id) const OVERRIDE;
   virtual bool IsCommandIdEnabled(int command_id) const OVERRIDE;
   virtual bool IsCommandIdVisible(int command_id) const OVERRIDE;
   virtual bool GetAcceleratorForCommandId(
      int command_id,
      ui::Accelerator* accelerator) OVERRIDE;
  virtual bool IsItemForCommandIdDynamic(int command_id) const OVERRIDE;
  virtual base::string16 GetLabelForCommandId(int command_id) const OVERRIDE;
  virtual base::string16 GetSublabelForCommandId(int command_id) const OVERRIDE;
  virtual void ExecuteCommand(int command_id, int event_flags) OVERRIDE;
  virtual void MenuWillShow(ui::SimpleMenuModel* source) OVERRIDE;

  //virtual void AttachToWindow(Window* window);
  virtual void Popup();

   scoped_ptr<ui::SimpleMenuModel> model_;
   ExoMenu* parent_;

   void InsertItemAt(int index, int command_id, const base::string16& label);
   void InsertSeparatorAt(int index);
  // void InsertCheckItemAt(int index,
  //                        int command_id,
  //                        const base::string16& label);
  // void InsertRadioItemAt(int index,
  //                        int command_id,
  //                        const base::string16& label,
  //                        int group_id);
void InsertSubMenuAt(int index,
                           int command_id,
                           const base::string16& label,
                           ExoMenu* menu);
  // void SetSublabel(int index, const base::string16& sublabel);
  // void Clear();
  int GetIndexOfCommandId(int command_id);
  // int GetItemCount() const;
  // int GetCommandIdAt(int index) const;
  // base::string16 GetLabelAt(int index) const;
  // base::string16 GetSublabelAt(int index) const;
  // bool IsItemCheckedAt(int index) const;
  // bool IsEnabledAt(int index) const;
  // bool IsVisibleAt(int index) const;
 protected:
 private:
  DISALLOW_COPY_AND_ASSIGN(ExoMenu);
protected:





};

} // namespace exo_shell

#endif // EXO_SHELL_BROWSER_EXO_MENU_H_
