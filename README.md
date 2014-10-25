thrust
======

Thrust is a cross-platform (Linux, OSX, Windows) application shell bindable from
any language. It is designed to ease the creation, packaging and distribution of
cross-platform native desktop appplication.

Thrust embeds Chromium Content API and exposes its API through a local JSON RPC
server listening on unix domain socket. Through the use of a language library,
developers can control Thrust and create shell window, sessions, menus, etc...

Thrust also come with support for the `<webview>` tag allowing the execution of
remote pages in a entirely secure setting.

Thrust will be used by next releases of Breach.

```
[Thurst Architecture]

              (Platform)                        (Client Implementation)
                                                                       
                                           #
                   +------------------+    #        +-----------------------+
                   |   Cocoa / Aura   |    #    +---|  shell3: (HTML/JS)    |
                   +---------+--------+    #    |  +-----------------------++
                             |             #    +--|  shell2: (HTML/JS)    |
+----------------+ +---------+--------+    #    | +-----------------------++
|                +-+   Thrust (C++)   +---------+-+  shell1: (HTML/JS)    |
|  Content  API  | +---------+--------+    #      +-----------------------+
|                |           |             #                 | (TCP/FS)      
|  (Blink / v8)  | +---------+--------+    #      +-----------------------+
|                | +   JSON RPC srv   +-----------+ Client App (any Lang) |
+----------------+ +------------------+    #      +-----------------------+
                                           #
```

### Using Thrust

To use thrust you need to rely on a binding library for your language of choice.
Libraries are currently available for `Go` and `NodeJS`.

Thrust is supported on `Linux`, `MacOSX` and `Windows`.

#### NodeJS

Simply install `node-thrust` as any other package. At `postinstall` a binary
image of `thrust` is downloaded for your platform (form this repository's 
[releases downloads](https://github.com/breach/thrust/releases))

```
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

### Building Thrust

You'll need to have `python` and `git` installed. You can then boostrap the
project with:
```
./scripts/boostrap.py                                
```

Build both the `Release` and `Debug` target with the following commands:
```
./scripts/update.py
./scripts/build.py
```

Note that `bootstrap.py` may take some time as it checks out `brightray` and
downloads `libchromiumcontent` for your platform.


### Getting Involved

- Mailing list: [breach-dev@googlegroups.com](https://groups.google.com/d/forum/breach-dev)
- IRC Channel: #breach on Freenode

