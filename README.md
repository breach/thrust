#### DEPRECATED - This package is no longer supporter or maintained.



![Logo Thrust](http://i.imgur.com/IviZAGZ.png)

**Chromium-based cross-platform / cross-language application framework**

*Thrust is `require/import`-able, it lets you distribute NodeJS, Go or Python GUI apps directly 
through their native package managers.*

Thrust is based on Chromium's Content Module and is supported on Linux, MacOSX and Windows:

![Cross-Platform Screenshots](http://i.imgur.com/7K98jyW.png)
*Screenshot of Thrust Getting Started example running on each major platform.*

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
  - [Scala](#scala)
  - [Clojure](#clojure)
  - [Perl](#perl)
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

```Javascript
require('node-thrust')(function(err, api) { 
  api.window({ root_url: 'https://google.com' }).show();
});
```

##### Library

- **node-thrust** [breach/node-thrust](https://github.com/breach/node-thrust/)

### Go

##### Getting Started

First download with `go get -u github.com/miketheprogrammer/go-thrust/`

```Go
```
package main

import (
	"github.com/miketheprogrammer/go-thrust/lib/dispatcher"
	"github.com/miketheprogrammer/go-thrust/lib/spawn"
	"github.com/miketheprogrammer/go-thrust/lib/bindings/window"
	"github.com/miketheprogrammer/go-thrust/lib/commands"
)

func main() {
	spawn.Run()
	size := commands.SizeHW{}
	opts := window.Options{
		RootUrl:  "http://google.com",
		Size:     size,
		Title:    "Demo window",
		HasFrame: true,
	}
	thrustWindow := window.NewWindow(opts)
	thrustWindow.Show()
	thrustWindow.Maximize()
	thrustWindow.Focus()
	dispatcher.RunLoop()
}

##### Library

- **go-thrust**: [miketheprogrammer/go-thrust](https://github.com/miketheprogrammer/go-thrust)

### Python

##### Getting Started

First install with `pip3 install pythrust [--user]` (requires Python3)

```Python
import asyncio, pythrust

loop = asyncio.get_event_loop()
api = pythrust.API(loop)

asyncio.async(api.spawn())
asyncio.async(api.window({ 'root_url': 'http://google.com' }).show())

loop.run_forever()
```

##### Library

- **pythrust** [breach/pythrust](https://github.com/breach/pythrust/)

### Scala

##### Getting Started

Include scala-thrust jar on your classpath. (Add to lib in your project.)

```Scala
import scala.concurrent.ExecutionContext.Implicits.global
import com.github.eklavya.thrust._

object Main extends App {
  Window.create("http://google.com").foreach { w =>
    w.show
    w.maximize
    w.openDevtools
    w.focus(true)
    w.onBlur(() => println("we were blurred"))
    w.onFocus(() => println("we were focused"))
    Menu.create("MyMenu").foreach { m =>
      val i = MenuItem("Item1", _ => println("Item1 was clicked"))
      m.addItem(i)
      m.popup(w)
    }
  }
}
```

##### Library

- **scala-thrust** [eklavya/scala-thrust](https://github.com/eklavya/scala-thrust/)

### Clojure

##### Getting Started

- [Installation](https://github.com/solicode/clj-thrust#installation)
- [Sample Project](https://github.com/solicode/clj-thrust#sample-project)

```clojure
(ns my-app.core
  (:require [clj-thrust.core :refer [create-process destroy-process]]
            [clj-thrust.window :as w]))

(let [process (create-process) ; `create-process` also takes path to Thrust directory
      window (w/create-window process
               :root-url "http://localhost:8080" ; URL to your web app
               :size {:width 400 :height 300})]
  (w/listen-closed window
    (fn [e]
      (destroy-process process))) ; Optionally call `(System/exit 0)` here.
  (w/show window)
  (w/focus window true))
```

##### Library

- **clj-thrust** [solicode/clj-thrust](https://github.com/solicode/clj-thrust)

### Perl

##### Getting Started

Install with `cpanm Thrust [--sudo]`

Simple command line test:

```
perl -MThrust -e 'Thrust->window->show->maximize->open_devtools->run'
```

Basic program

```Perl
use Thrust;

my $t = Thrust->new;

my $w = $t->window(
          root_url => 'data:text/html,Hello World!',
          title => 'My App',
          size => { width => 800, height => 600 },
        );

$w->on(closed => sub { exit });

$w->show;

$t->run; ## enter event loop
```

##### Library

- [metacpan](https://metacpan.org/pod/Thrust)
- [github](https://github.com/hoytech/Thrust)


***
## API Reference

The API reference as well as links to specific language bindings documentations 
are availble in the [docs/](https://github.com/breach/thrust/tree/master/docs)
 directory.

***
## Architecture

```
[Thrust Architecture]

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

##### Getting Involved
No longer maintained actively.

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
- [x] **remote** thrust specific IPC mechanism for client/server communication
- [x] **proxy** enable traffic proxying (Tor, header injection, ...)
- [ ] **tray icon** tray icon native integration
- [ ] **protocol** specific protocol registration (`file://`, ...)

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

