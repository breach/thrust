session
=======

The `session` object provides an API to manage sessions for a window (cookies,
storage).

#### Constructor

- `off_the_record` if true windows using this session won't write to disk
- `path` path under which session information should be stored (cache, storage)
- `cookie_store` whether or not to use a custom cookie store

#### Method: `visitedlink_add`

- `url` a link url

Adds the specified url to the list of visited links for this session

#### Method: `visitedlink_clear`

Clears the visited links storage for this session

#### Accessor: `off_the_record` 

Returns whether the session is off the record or not

#### Remote Method: `cookies_load`

Retrieves all the cookies from the custom cookie store

#### Remote Method: `cookies_load_for_key`

- `key` domain key to retrieve cookie for

Retrieves the cookies for the specified domain key. 

#### Remote Method: `cookies_flush`

Flush all cookies to permanent storage

#### Remote Method: `cookies_add`

- `cookie` 
  - `source` the source url
  - `name` the cookie name
  - `value` the cookie value
  - `domain` the cookie domain
  - `path` the cookie path
  - `creation` the creation date
  - `expiry` the expiration date
  - `last_access` the last time the cookie was accessed
  - `secure` is the cookie secure
  - `http_only` is the cookie only valid for HTTP
  - `priority` internal priority information

Add the specified cookie to the custom cookie store.

#### Remote Method: `cookies_update_access_time`

- `cookie` 
  - `source` the source url
  - `name` the cookie name
  - `value` the cookie value
  - `domain` the cookie domain
  - `path` the cookie path
  - `creation` the creation date
  - `expiry` the expiration date
  - `last_access` the last time the cookie was accessed
  - `secure` is the cookie secure
  - `http_only` is the cookie only valid for HTTP
  - `priority` internal priority information

Updates the `last_access` time for the cookie specified

#### Remote Method: `cookies_delete`

- `cookie` 
  - `source` the source url
  - `name` the cookie name
  - `value` the cookie value
  - `domain` the cookie domain
  - `path` the cookie path
  - `creation` the creation date
  - `expiry` the expiration date
  - `last_access` the last time the cookie was accessed
  - `secure` is the cookie secure
  - `http_only` is the cookie only valid for HTTP
  - `priority` internal priority information

Removes the specified cookie from the custom cookie store.


#### Remote Method: `cookies_force_keep_session_state`

Informs the cookie store that it should keep session cookie across restart.

