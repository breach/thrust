ExoBrowser
==========

The ExoBrowser is a scriptable platform designed to ease the experimentation 
with new concepts for the Web Browser.

The ExoBrowser exposes its API (parts of the Chromium Content API + 
a Simple View Model) in Javascript and enables the implementation of a fully 
functional browser entirely out of it (as a Javascript/HTML/CSS app)

The ExoBrowser is used by Breach.

#### The ExoBrowser Architecture

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

[ExoBrowser Architecture]

             (Platform)                  #   (Browser Implementation)
                                         #
                   +------------------+  #
                   |  Cocoa/Win/GTK+  |  #
                   +---------+--------+  #
                             |           #
+----------------+ +---------+--------+  #  +-----------------------+
|  Content API   +-+ ExoBrowser (C++) |  #  |  Web Views (HTML/JS)  |
+----+-----------+ +--------------+---+  #  +-----------------------+
     |                   (JS API) |      #             | (TCP)      
+----+---+  +----+ +--------------|---+  #  +-----------------------+
| Webkit +--+ v8 +-+    NodeJS    +---+-----+   Local Server (JS)   |
+--------+  +----+ +------------------+  #  +-----------------------+
```

### Getting the Source Code & Building the ExoBrowser

See [Building the ExoBrowser](https://github.com/breach/exo_browser/wiki/Building-the-ExoBrowser)

### Hacking with the ExoBrowser

See [ExoBrowser JS API Specification](https://github.com/breach/exo_browser/blob/master/API.md)

### Get in Touch

- Mailing list: [breach-dev@googlegroups.com](https://groups.google.com/d/forum/breach-dev)
- IRC Channel: #breach on Freenode

