![Logo Thrust](http://i.imgur.com/DwFKI0J.png)

**Chromium-based cross-platform / cross-language application framework**

*Thrust is `require/import`-able, it lets you distribute NodeJS, Go or Python GUI apps directly 
through their native package managers.*

Thrust is based on Chromium's Content Module and is supported on Linux, MacOSX and Windows:

![Cross-Platform Screenshots](http://i.imgur.com/7K98jyW.png)
*Screenshot of Thrust Getting Started example running on each major platform.*

Thrust will be powering the next versions of [Breach](http://breach.cc)

To better understand what Thrust can do, check out **[JankyBrowser](https://gist.github.com/morganrallen/f07f59802884bcdcad4a)** by 
@morganrallen, the cross-platform browser that fits in a gist:
```
npm install -g \
  https://gist.github.com/morganrallen/f07f59802884bcdcad4a/download
```

#### Table of Contents
- [Language bindings](#language-bindings)
  - [NodeJS](#nodejs)
  - [Go](#go)
  - [Python](#python)
- [API Reference](#api-reference)
- [Architecture](#architecture)
- [Community](#community)
  - [Request for API](#request-for-api)
  - [List of Thrust Users](#list-of-thrust-users)
  - [Thrust 7.5k Contest](#thrust75k-contest)
  - [Getting Involved](#getting-involved)
- [Features & Roadmap](#features--roadmap)
- [Building Thrust from Sources](#building-thrust-from-sources)

***
## Language bindings

Thrust's binary distribution exposes its API on the standard IO and language
 specific library packages automatically download the binary distribution at 
installation. Thrust is based on Chromium's content module and uses web-pages 
as its GUI.

All these Getting Started example work as is on each major platform (MacOSX,
Windows, Linux)

### NodeJS

##### Getting Started

First install with `npm install node-thrust`

```
require('node-thrust')(function(err, api) { 
  api.window({ root_url: 'https://breach.cc' }).show();
});
```

##### Library

- **node-thrust** [breach/node-thrust](https://github.com/breach/node-thrust/)

### Go

##### Getting Started

First download with `go get -u github.com/miketheprogrammer/go-thrust/`

```
package main

import (
  "github.com/miketheprogrammer/go-thrust/dispatcher"
  "github.com/miketheprogrammer/go-thrust/spawn"
  "github.com/miketheprogrammer/go-thrust/window"
)
 
func main() {
  spawn.Run()
  thrustWindow := window.NewWindow("http://breach.cc/", nil)
  thrustWindow.Show()
  thrustWindow.Maximize()
  thrustWindow.Focus()
  dispatcher.RunLoop()
}
```

##### Library

- **go-thrust**: [miketheprogrammer/go-thrust](https://github.com/miketheprogrammer/go-thrust)

### Python

##### Getting Started

First install with `pip3 install pythrust [--user]` (requires Python3)

```
import asyncio, pythrust

loop = asyncio.get_event_loop()
api = pythrust.API(loop)

asyncio.async(api.spawn())
asyncio.async(api.window({ 'root_url': 'http://breach.cc' }).show())

loop.run_forever()
```

##### Library

- **pythrust** [breach/pythrust](https://github.com/breach/pythrust/)

***
## API Reference

The API reference as well as links to specific language bindings documentations 
are availble in the [docs/](https://github.com/breach/thrust/tree/master/docs)
 directory.

***
## Architecture

```
[Thurst Architecture]

          (Platform)           [stdio]      (Your Implementation)
                                                                          
                                  #
               +--------------+   #       +-----------------------+  | 
               | Cocoa / Aura |   #   +---|    win3: (HTML/JS)    |  |
               +-------+------+   #   |  +-----------------------++  |
                       |          #   +--|    win2: (HTML/JS)    |   | cli
+------------+ +-------+------+   #   | +-----------------------++   |
|            +-+ thrust (C++) +-------+-+    win1: (HTML/JS)    |    |
| ContentAPI | +-------+------+   #     +-----------------------+    |
|            |         |          #                | (TCP/FS)      
| (Blink/v8) | +-------+------+   #     +-----------------------+    |
|            | + JSON RPC srv +---------+ Client App (any Lang) |    | srv
+------------+ +--------------+   #     +-----------------------+    |
                                  #
```

***
## Community

##### Request for API

- List of API needed by various projects on Thrust: 
[Request for API](https://github.com/breach/thrust/wiki/Request-for-API)

##### List of Thrust Users 

- List of people relying on Thrust: 
[List of Thrust Users](https://github.com/breach/thrust/wiki/List-of-Thrust-Users)

##### Thrust7.5k Contest

- 7.5k max Browser implementation based on Thrust. *Coming soon*. Ping 
dev@breach.cc if interested! (Credits to @morganrallen for the awesome idea) 

##### Getting Involved

- Mailing list: [breach-dev@googlegroups.com](https://groups.google.com/d/forum/breach-dev)
- IRC Channel: #breach on Freenode

***
## Features & Roadmap

- [x] **window creation** create, show, close resize, minimize, maximize, ...
- [x] **node.js, go** node.js and go bindings libraries
- [x] **window events** close, blur, focus, unresponsive, crashed
- [x] **cross-platform** equivalent support on `MacOSX`, `Windows` and `Linux`
- [x] **sessions** off the record, custom storage path, custom cookie store
- [x] **kiosk** kiosk mode
- [x] **application menu** global application menu (MacOSX, X11/Unity)
- [x] **webview** webview tag (secure navigation, tabs management)
- [x] **frameless** frameless window and draggable regions
- [x] **python** python bindings library
- [ ] **tray icon** tray icon native integration
- [ ] **remote** thrust specific IPC mechanism for client/server communication
- [ ] **protocol** specific protocol reigstration (`fille://`, ...)
- [ ] **proxy** enable traffic proxying (Tor, header injection, ...)

***
## Building Thrust from Sources

You will generally don't need to build thrust yourself. A binary version of 
thrust should be automatically fetched by the library you're reyling on at 
installation.

To build thrust, you'll need to have `python 2.7.x` and `git` installed. You can 
then boostrap the project with:
```
./scripts/boostrap.py                                
```

Build both the `Release` and `Debug` targets with the following commands:
```
./scripts/update.py
./scripts/build.py
```

Note that `bootstrap.py` may take some time as it checks out `brightray` and
downloads `libchromiumcontent` for your platform.

