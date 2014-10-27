thrust
======

Thrust enables you to create rich cross-platform (`MacOSX`, `Windows` and 
`Linux`) desktop applications from the language of your choice (Go, NodeJS, 
Python, Java, ...). Thrust is based on Chromium and uses web-pages as its GUI, 
so you can see it as a minimal Chromium browser controlled by your code.

Thrust lets you create and manage native windows, load web contents, manage 
native OS integrations (dock, menus, ...) through a standard IO API.

Contrary to atom-shell or node-webkit, thrust does not rely on or embed 
NodeJS, making it usable directly from your usual programming environment 
(simple `require` in NodeJS, `pip` package, Go dependency, ...)

Thrust is used by [Breach](http://breach.cc)

```
[Thurst Architecture]

              (Platform)                [stdio]    (Client Implementation)
                                                                            
                                           #
                   +------------------+    #        +-----------------------+
                   |   Cocoa / Aura   |    #    +---|    win3: (HTML/JS)    |
                   +---------+--------+    #    |  +-----------------------++
                             |             #    +--|    win2: (HTML/JS)    |
+----------------+ +---------+--------+    #    | +-----------------------++
|                +-+   thrust (C++)   +---------+-+    win1: (HTML/JS)    |
|  Content  API  | +---------+--------+    #      +-----------------------+
|                |           |             #                 | (TCP/FS)      
|  (Blink / v8)  | +---------+--------+    #      +-----------------------+
|                | +   JSON RPC srv   +-----------+ Client App (any Lang) |
+----------------+ +------------------+    #      +-----------------------+
                                           #
```

### Using thrust

To use thrust you need to rely on a binding library for your programming 
language.  Libraries are currently available for `Go` and `NodeJS`. 

If you want to create a binding library for another language, please get in 
touch ASAP (We're especially looking for people willing to contribute for 
Python, Ruby, Java, Rust).

Thrust is supported on `MacOSX`, `Windows` and `Linux`.

#### NodeJS

To use thrust with NodeJS, you just need to add `node-thrust` as a dependency.
Contrary to `atom-shell` or `node-webkit`, you can rely on your vanilla NodeJS
installation and don't need to recompile native addons with custom binary images.

Additionally you can use `npm` to distribute your application (it only has to 
depend on the `node-thrust` package).

```
npm install node-thrust
```

At `postinstall` a binary image of thrust is automatically downloaded for your 
platform (form this repository's [releases](https://github.com/breach/thrust/releases))

```
// test.js
require('node-thrust')(function(err, api) {
  api.window({
    root_url: 'https://www.google.com/',
    size: {
      width: 1024,
      height: 768
    }
  }).show(function(err) {
    console.log('WINDOW CREATED');
  })
});
```

See [breach/node-thrust](https://github.com/breach/node-thrust) for more details.

#### Go

```
[TODO]
```

See [miketheprogrammer/go-thrust](https://github.com/miketheprogrammer/go-thrust) for more details.

### Roadmap

- [x] **window creation** create, show, close resize, minimize, maximize, ...
- [x] **window events** close, blur, focus, unresponsive, crashed
- [x] **cross-platform** equivalent support on `MacOSX`, `Windows` and `Linux`
- [x] **sessions** off the record, custom storage path, custom cookie store
- [x] **application menu** global application menu (MacOSX, Unity)
- [ ] **webview** webview tag (secure navigation, tabs management)
- [ ] **tray icon** tray icon native integration
- [ ] **remote** thrust specific IPC mechanism for client/server communication
- [ ] **protocol** specific protocol reigstration (`fille://`, ...)

### Building thrust

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


### Getting Involved

- Mailing list: [breach-dev@googlegroups.com](https://groups.google.com/d/forum/breach-dev)
- IRC Channel: #breach on Freenode

