Exo
====

See [The Experimentation Platform to Build a Next Generation Browser](https://github.com/spolu/exo/wiki/The-Experimentation-Platform-to-Build-a-Next-generation-Web-Browser)

We set out to build a new Browser and more importantly a new Experimentation Platform to build a better Browser for today's Web.

- **Free** Open-source, and most importantly freed from any assumptions we have about web browsing
- **Usable** Disruptive but only if it's easy to use and easy to learn
- **Hackable** Empower users with the ability to easily change or extend the behaviour of their browser

Before releasing a fully-fledged browser, our goal is to run a set of experiments to explore specific concepts we find particularly interesting: 

- *Experiment 1*: [Stacked Navigation](https://github.com/spolu/exo/wiki/The-Experimentation-Platform-to-Build-a-Next-generation-Web-Browser/#stacked-navigation)
- *Experiment 2*: [Synchronized Sessions](https://github.com/spolu/exo/wiki/The-Experimentation-Platform-to-Build-a-Next-generation-Web-Browser/#synchronized-sessions)

Each experiments will be released as a specialized browser, whose feature set is focused on the related experiment. To make this possible, we realized early on that we needed a proper testbed to try out concepts, iterate and validate them easily. Using platform-specific UI frameworks and C++ code was probably not going to cut it. So we came up with the concept of the **Scriptable Browser**. We call it the *ExoBrowser*. 

#### ExoBrowser Architecture

```txt
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

See [Exp.1: Stacked Navigation](https://github.com/spolu/exo/wiki/Exp.1:-Stacked-Navigation)

![Stacked Navigation Screenshot](https://f.cloud.github.com/assets/15067/1078722/b7909be6-1531-11e3-837b-1764eab48739.png)

#### Exp.2: Synchronized Sessions

See [Exp.2: Synchronized Sessions](https://github.com/spolu/exo/wiki/Exp.2:-Synchronized-Sessions)

### Getting the Source Code & Building Exo

[Building Exo](https://github.com/spolu/exo/wiki/Building-Exo)

### Hacking

[ExoBrowser JS API Specification](https://github.com/spolu/exo/blob/master/API.md)

### Get in Touch

- Mailing list: [exobrowser-dev@googlegroups.com](https://groups.google.com/forum/#!forum/exobrowser-dev)
- IRC Channel: #exobrowser on Freenode
