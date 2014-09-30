ExoShell
========

The ExoShell is a scriptable platform designed to ease the experimentation 
with new concepts for the Web Browser.

The ExoShell exposes its API (parts of the Chromium Content API + 
a Simple View Model) in Javascript and enables the implementation of a fully 
functional browser entirely out of it (as a Javascript/HTML/CSS app)

The ExoShell is used by Breach.

#### The ExoShell Architecture

```
[Chromium Architecture]
  
  (Platform)        #   (Browser Implementation)
+----------------+  #  +-----------------------+
|  Content API   +-----+     Chrome (C++)      |
+----+-----------+  #  +-----------------------+
     |              #     |       |        |
+----+---+  +----+  #  +-----+ +-----+ +-------+
| Webkit +--+ v8 |  #  | GTK | | Win | | Cocoa |
+--------+  +----+  #  +-----+ +-----+ +-------+

`vs.`

[ExoShell Architecture]

             (Platform)                  #   (Browser Implementation)
                                         #
                   +------------------+  #
                   |  Cocoa/Win/GTK+  |  #
                   +---------+--------+  #
                             |           #
+----------------+ +---------+--------+  #  +-----------------------+
|  Content API   +-+  ExoShell (C++)  |  #  |  Web Views (HTML/JS)  |
+----+-----------+ +--------------+---+  #  +-----------------------+
     |                   (JS API) |      #             | (TCP)      
+----+---+  +----+ +--------------|---+  #  +-----------------------+
| Webkit +--+ v8 +-+    NodeJS    +---+-----+   Local Server (JS)   |
+--------+  +----+ +------------------+  #  +-----------------------+
```

### Getting the Source Code & Building the ExoBrowser

See [Building the ExoShell](https://github.com/breach/exo_shell/wiki/Building-the-ExoBrowser)

### Hacking with the ExoShell

See [ExoShell JS API Specification](https://github.com/breach/exo_shell/blob/master/API.md)

### Getting Involved

- Mailing list: [breach-dev@googlegroups.com](https://groups.google.com/d/forum/breach-dev)
- IRC Channel: #breach on Freenode

