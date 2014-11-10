menu
====

The `menu` object provides an API to create native global application menu 
(MacOSX and X11/Unity only) and native context menus.

Window specific menus on other platform are meant to be handled using client
side code.

#### Constructor

#### Method: `add_item`

- `command_id` the label command id (see `execute`)
- `label` the item label

Adds a standard item to the menu

#### Method: `add_check_item`

- `command_id` the label command id (see `execute`)
- `label` the item label

Adds a check item to the menu

#### Method: `add_radio_item`

- `command_id` the label command id (see `execute`)
- `label` the item label
- `group_id` radio group

Adds a radio item to the menu

#### Method: `add_separator`

Adds a separator to the menu

#### Method: `set_checked`

- `command_id` the command of the item to alter
- `value` true or false

Sets an item checked or unchecked

#### Method: `set_enabled`

- `command_id` the command of the item to alter
- `value` true or false

Sets an item enabled or disabled

#### Method: `set_visible`

- `command_id` the command of the item to alter
- `value` true or false

#### Method: `set_accelerator`

- `command_id` the command id of the item to alter
- `accelerator` accelerator string

Sets the accelerator string for the menu item

Sets an item visible or invisible

#### Method: `add_submenu`

- `menu_id` the menu id to add as submenu
- `label` label for the submenu
- `command_id` command id for the submenu item

Adds an other menu as submenu of this menu

#### Method: `clear`

Clears the menu of all its items

#### Method: `popup`

- `window_id` the window id on which to popup the menu

Popup the menu as a context menu under the current mouse position for the window
specified by its id.

#### Method: `set_application_menu`

Sets this menu as the global application menu on MacOSX and X11/Unity

#### Remote Method: `execute`

- `command_id` the command id of the item that was clicked
- `event_flags` event flag integer

Called when a menu item is clicked
