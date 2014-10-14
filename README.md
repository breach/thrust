Thrust
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

#### Thrust Architecture

```
[Thurst Architecture]
             (Platform)                        (Client Implementation)
                                                                       
                                          #
                   +------------------+   #      +-----------------------+
                   |  Cocoa/Win/GTK+  |   #  +---|  shell3: (HTML/JS)    |
                   +---------+--------+   #  |  +-----------------------++
                             |            #  +--|  shell2: (HTML/JS)    |
+----------------+ +---------+--------+   #  | +-----------------------++
|  Content API   +-+   Thrust (C++)   +------+-+  shell1: (HTML/JS)    |
+----+-----------+ +---------+--------+   #    +-----------------------+
     |                       |            #               | (TCP/FS)      
+----+---+  +----+ +---------+--------+   #    +-----------------------+
| Blink  +--+ v8 +-+   JSON RPC srv   +--------+ Client App (any Lang) |
+--------+  +----+ +------------------+   #     +-----------------------+
                                          #
```

### Testing

Thrust currently is testable only manually by running the exo_shell executable
and runnin thrust-node library test.js file.

### Getting Involved

- Mailing list: [breach-dev@googlegroups.com](https://groups.google.com/d/forum/breach-dev)
- IRC Channel: #breach on Freenode

