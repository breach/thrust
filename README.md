ExoBrowser
==========

The ExoBrowser is a scriptable platform designed to ease the experimentation with new concepts for the Web Browser.

- **Free** Open-source, and Free
- **Usable** Innovative but only if it's easy to use and easy to learn
- **Hackable** Lets users easily change or extend the behaviour of their browser

The ExoBrowser exposes its API (parts of the Chromium Content API + a Simple View Model) in Javascript and enables the implementation of a fully functional browser entirely out of it (as a Javascript/HTML/CSS app)

See [The Experimentation Platform to Build a Next Generation Browser](http://spolu.github.io/exo_browser/2013/09/05/the-experimentation-platform-to-build-a-next-generation-web-browser.html)

We're currently focused on running a set of experiments on top of the ExoBrowser to explore new concepts:

- *Experiment 1*: Stacked Navigation
- *Experiment 2*: Synchronized Sessions

#### Downloads

See [Downloads](http://spolu.github.io/exo_browser/downloads/)

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
+----------------+ +------------------+  #  +-----------------------+
|  Content API   +-+ ExoBrowser (C++) |-----+  Web Views (HTML/JS)  |
+----+-----------+ +--------------+---+  #  +-----------------------+
     |                   (JS API) |      #             | (Net)      
+----+---+  +----+ +--------------|---+  #  +-----------------------+
| Webkit +--+ v8 +-+    NodeJS    +---+-----+   Local Server (JS)   |
+--------+  +----+ +------------------+  #  +-----------------------+
```

### Getting the Source Code & Building the ExoBrowser

See [Building the ExoBrowser](https://github.com/spolu/exo_browser/wiki/Building-the-ExoBrowser)

### Hacking with the ExoBrowser

See [ExoBrowser JS API Specification](https://github.com/spolu/exo_browser/blob/master/API.md)

### Get in Touch

- Mailing list: [exobrowser-dev@googlegroups.com](https://groups.google.com/forum/#!forum/exobrowser-dev)
- IRC Channel: #exobrowser on Freenode
- Twitter: [@ExoBrowser](https://twitter.com/ExoBrowser)

