`<webview>`
===========

The `<webview>` tag is available in the top-level document of any Thrust windows
and lets client code embed untrusted content securely and efficiently. The 
`<webview>` runs in its own separate process and has stricter permissions than 
an `<iframe>`.

The `<webview>` was introduced to enable secure and efficient implementation of
web browsers using Thrust (think of a `<webview>` as a Tab). It should be used
anytime anytime you want to display untrusted web content.

The `<webview>` provides an extensive API and set of custom DOM Events to 
control the embedded web content.

### Example

```
<webview id="tab"
         src="https://breach.cc" 
         style="opacity: 0.5; width: 400px; height: 300px">

<script>
  onload = function() {
    var wv = document.getElementById("tab");
    wv.addEventListener('did-start-loading', function() {
      console.log('Loading... ' + wv.getAttribute('src'));
    });
    wv.addEventListener('did-stop-loading', function() {
      console.log('Loaded ' + wv.getAttribute('src'));
    });
  }
</script>
```

#### Attribute: `src`

```
<webview src="https://www.breach.cc/"></webview>
```

Returns the visible URL. Writing to this attribute navigates the webview.

#### Attribute: `autosize`

```
<webview autosize="on" 
         minwidth="800" minheight="600"
         maxwidth="1600" maxheight="900"></webview>
```

If set, the webview container will automatically resize within the bounds
specified by `minwidth`, `minheight`, `maxwidth`, `maxheight`.

####  Method: `go`

- `index` relative index

Go back or forward by relative index within the web content navigation history

#### Method: `back`

Go back in the web content navigation history. Equivalent to `go(-1)`.

#### Method: `forward`

Go forward in the web content navigation history. Equivalent to `go(1)`.

#### Method: `canGoBack`

Returns whether the web content can go back.

#### Method: `canGoForward`

Returns whether the web content can go forward.

#### Method: `loadUrl`

- `url` the url to navigate to

Navigates to the specified URL.

#### Method: `reload`

- `ignore_cache` whether to ignore cache or not (by default `false`)

Reload the current page, optionally ignoring cache

#### Method: `stop`

Stops the loading of the current page

#### Method: `getProcessId`

Returns the process id of the renderer associated with this embbeded web content

#### Method: `getZoom`

Returns the current zoom factor

#### Method: `setZoom`

- `zoom_factor` the zoom factor to apply

Sets the zoom factor for the embedded web content.

#### Method: `find`

- `request_id` the find request id (use new to start new find)
- `search_text` the text to search for
- `options` 
  - `forward` look forward or backward (default `true`)
  - `match_case` match the case of `search_text` (default `false`)
  - `find_next` whether it is a continued search (default `false`)
  - `word_start` only match word start (default: `false`)
  - `medial_capital_as_word_start` consider medial capital as word start (default: `false`)

Proceed to search for `search_text` within the embedded web content

#### Method: `stopFinding`

- `action` "clear", "keep" or "activate"

Stops the current find request and perform the specified action

#### Method: `insertCSS`

- `css` the css to inject

Injects the specified CSS code within the embedded web content

#### Method: `executeScript`

- `script` the script to inject

Executes the specified script within the embedded web contents script context

#### Method: `openDevTools`

Open the DevTools window associated with the embedded web content

#### Method: `closeDevTools`

Closes the DevTools window associated with the embedded web content

#### Method: `isDevToolsOpened`

Returns whether the DevTools window is opened for the embedded web content

#### Method: `getTitle`

Returns the title associated with the embedded web content

#### Event: `did-fail-load`

- `url` the current url
- `is_top_level` whether this event is associated with the top-level frame
- `error_code` the error code of the failure
- `error-description` the error description of the failure

Emitted when the embedded web content failed to load

#### Event: `did-finish-load`

- `url` the current url
- `is_top_level` whether this event is associated with the top-level frame

Emitted when a frame finished loading within the embedded web content.

#### Event: `did-start-loading`

Emitted when the embedded web content starts loading

#### Event: `did-stop-loading`

Emitted when the embedded web content has finished loading

#### Event: `did-get-redirect-request`

- `current_url` the current URL
- `new_url` the new URL to redirect to
- `is_top_level` whether the request is for the top-level frame

Emitted whenever a frame receives a redirect request

#### Event: `console`

- `level` the console message level
- `message` the message
- `line` the line at which the message was generated
- `source_id` the id of the source of the message

Emitted when the embedded content attempts to log somthing

#### Event: `new-window`

- `target_url` the target URL
- `frame_name` the name of the target frame
- `window_container_type` the type of container if a popup
- `disposition` the desired disposition

Emitted when the embedded web content attempts to load a new window or popup

#### Event: `close`

Emitted when the webview is closed

#### Event: `crashed`

Emitted when the webview renderer crashes

#### Event: `destroyed`

Emitted when the webview is destroyed

### Event: `dialog`

Emitted when the embedded web content attemps to display a dialog

- `origin_url` the URL of the origin of the dialog
- `accept_lang` hint on the languages accepted
- `message_type` the dialog message type
- `message_text` the dialog message text
- `default_prompt_text`  the default prompt text
- `ok(response)` function to call once the dialog is closed with a user response
- `cancel()` function to call once the dialog is cancelled

Emitted whenever the embedded content attempts to display a dialog. The default
behaviour is to cancel the dialog.

#### Event: `title-set`

- `title` the new title
- `explicit_set` whether it was explicitly set by the page

Emitted when the title associated with the embedded web content is changed.





