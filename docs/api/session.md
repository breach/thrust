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

#### Method: `proxy_set`

- `rules` proxy rules string

Sets the specified proxy rules (as string) for the current session

```
 proxy-uri = [<proxy-scheme>"://"]<proxy-host>[":"<proxy-port>]
 proxy-uri-list = <proxy-uri>[","<proxy-uri-list>]
 url-scheme = "http" | "https" | "ftp" | "socks"
 scheme-proxies = [<url-scheme>"="]<proxy-uri-list>
 proxy-rules = scheme-proxies[";"<scheme-proxies>]
```

For example:
```
"http=foopy:80;ftp=foopy2"  -- use HTTP proxy "foopy:80" for http://
                               URLs, and HTTP proxy "foopy2:80" for
                               ftp:// URLs.
"foopy:80"                  -- use HTTP proxy "foopy:80" for all URLs.
"foopy:80,bar,direct://"    -- use HTTP proxy "foopy:80" for all URLs,
                               failing over to "bar" if "foopy:80" is
                               unavailable, and after that using no
                               proxy.
"socks4://foopy"            -- use SOCKS v4 proxy "foopy:1080" for all
                               URLs.
"http=foop,socks5://bar.com -- use HTTP proxy "foopy" for http URLs,
                               and fail over to the SOCKS5 proxy
                               "bar.com" if "foop" is unavailable.
"http=foopy,direct://       -- use HTTP proxy "foopy" for http URLs,
                               and use no proxy if "foopy" is
                               unavailable.
"http=foopy;socks=foopy2   --  use HTTP proxy "foopy" for http URLs,
                               and use socks4://foopy2 for all other
                               URLs.
```

#### Method: `proxy_clear`

Clears the proxy rules string for this session

#### Accessor: `is_off_the_record` 

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

