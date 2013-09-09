ExoBrowser
==========

The ExoBrowser is a scriptable platform designed to ease the experimentation with new concepts for the Web Browser.

- **Free** Open-source, and most importantly freed from any assumptions we have about web browsing
- **Usable** Disruptive but only if it's easy to use and easy to learn
- **Hackable** Empower users with the ability to easily change or extend the behaviour of their browser

The ExoBrowser exposes its API (parts of the Chromium Content API + a Simple View Model) in Javascript and enables the implementation of a fully functional browser entirely out of it (as a Javascript/HTML/CSS app)

See [The Experimentation Platform to Build a Next Generation Browser](https://github.com/spolu/exo_browser/wiki/The-Experimentation-Platform-to-Build-a-Next-generation-Web-Browser)

We're currently focused on running a set of experiments on top of the ExoBrowser to explore new concepts:

- *Experiment 1*: Stacked Navigation
- *Experiment 2*: Synchronized Sessions

Each experiments will be released as a specialized browser, whose feature set is focused on the related experiment. 

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

#### Exp.1: Stacked Navigation

See [Exp.1: Stacked Navigation](https://github.com/spolu/exo_browser/wiki/Exp.1:-Stacked-Navigation)

![Stacked Navigation Screenshot](https://f.cloud.github.com/assets/15067/1078722/b7909be6-1531-11e3-837b-1764eab48739.png)

Quick Intro to the Experiment: [[slides]](http://slid.es/stanislaspolu/exo_browser-exp1)


#### Exp.2: Synchronized Sessions

See [Exp.2: Synchronized Sessions](https://github.com/spolu/exo_browser/wiki/Exp.2:-Synchronized-Sessions)

### Getting the Source Code & Building the ExoBrowser

See [Building Exo](https://github.com/spolu/exo_browser/wiki/Building-the-ExoBrowser)

### Hacking with the ExoBrowser

See [ExoBrowser JS API Specification](https://github.com/spolu/exo_browser/blob/master/API.md)

### Get in Touch

- Mailing list: [exobrowser-dev@googlegroups.com](https://groups.google.com/forum/#!forum/exobrowser-dev)
- IRC Channel: #exobrowser on Freenode
- Twitter: [@ExoBrowser](https://twitter.com/ExoBrowser)

