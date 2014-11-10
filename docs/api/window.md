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

#### show

Makes the window visible

#### focus

- `focus` wether to focus or blur the window

Focuses or blur the window depending on the value of `focus`

#### maximize

Maximizes the window
