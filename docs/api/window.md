window
======

The `window` object provides an API to show and interact with native windows
in charge of executing client side code.

#### Constructor

- `root_url` the url to load as top-level document for the window
- `size`
  - `width` the initial window width
  - `height` the initial window height
- `title` the window title
- `icon_path` absolute path to a `PNG` or `JPG` icon file for the window
- `has_frame` creates a frameless window if `true`
- `session_id` the id of the session to use for this window

#### Event: `closed`

Emitted when the window is closed

#### Event: `blur`

Emitted when the window loses focus

#### Event: `focus`

Emitted when the window gains focus

#### Event: `unresponsive`

Emitted when the window renderer become unresponsive

#### Event: `responsive`

Emitted when the window renderer regains responsiveness

#### Event: `worker_crashed`

Emitted when the window renderer crashed

#### Event: `remote`

- `message` the message sent

Emitted when a remote event is sent by the window

#### Method: `show`

Makes the window visible

#### Method: `focus`

- `focus` whether to focus or blur the window

Focuses or blur the window depending on the value of `focus`

#### Method: `maximize`

Maximizes the window

#### Method: `minimize`

Minimizes the window

#### Method: `restore`

Restores a minimized window

#### Method: `set_title`

- `title` the title to set

Sets the title of a window

#### Method: `set_fullscreen`

- `fullscreen` whether to set the window fullscreen or not

Makes the window enter or leave fullscreen

#### Method: `set_kiosk`

- `kiosk` whether to set the window in kiosk mode

Makes the window enter or leave kiosk mode

#### Method: `open_devtools`

Opens the DevTools for this window's main document

#### Method: `close_devtools`

Closes the DevTools for this window's main document

#### Method: `move`

- `x` the new x position 
- `y` the new y position

Moves the window to the specified position

#### Method: `resize`

- `width` the new window width
- `height` the new window height

#### Method: `remote`

- `message` the message to send

Sends a remote message to the window main javascript context

#### Accessor: `is_closed`

Returns wether the window has been closed or not (can't be reopened)

#### Accessor: `size`

Returns the size of the window

#### Accessor: `position`

Returns the position of the window

#### Accessor: `is_maximized`

Returns whether the window is maximized or not

#### Accessor: `is_minimized`

Returns whether the window is minimized or not

#### Accessor: `is_fullscreen`

Returns whether the window is in fullscreen mode or not

#### Accessor: `is_kiosed`

Returns whether the window is in kiosk mode or not

#### Accessor: `is_devtools_opened`

Returns whether the window's main document has its DevTools opened or not
