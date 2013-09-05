Exo
====


Exo is a new Browser and more importantly a new Experimentation Platform to build a better Browser for today's Web.

- **Free** Open-source, and most importantly freed from any assumptions we have about web browsing
- **Usable** Disruptive but only if it's easy to use and easy to learn
- **Hackable** Empower users with the ability to easily change or extend the behaviour of their browser

Exo is scriptable, it is based on the ExoBrowser API (Chromium Content API + Simple View Model exposed in Javascript) and is entirely written in Javsascript.

See [The Experimentation Platform to Build a Next Generation Browser](https://github.com/spolu/exo/wiki/The-Experimentation-Platform-to-Build-a-Next-generation-Web-Browser)

Before releasing a fully-fledged browser, our goal is to run a set of experiments to explore specific concepts we find particularly interesting: 

- *Experiment 1*: Stacked Navigation
- *Experiment 2*: Synchronized Sessions

Each experiments will be released as a specialized browser, whose feature set is focused on the related experiment. 

#### ExoBrowser Architecture

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

See [Exp.1: Stacked Navigation](https://github.com/spolu/exo/wiki/Exp.1:-Stacked-Navigation)

![Stacked Navigation Screenshot](https://f.cloud.github.com/assets/15067/1078722/b7909be6-1531-11e3-837b-1764eab48739.png)

#### Exp.2: Synchronized Sessions

See [Exp.2: Synchronized Sessions](https://github.com/spolu/exo/wiki/Exp.2:-Synchronized-Sessions)

### Getting the Source Code & Building Exo

See [Building Exo](https://github.com/spolu/exo/wiki/Building-Exo)

### Hacking

See [ExoBrowser JS API Specification](https://github.com/spolu/exo/blob/master/API.md)

### Get in Touch

- Mailing list: [exobrowser-dev@googlegroups.com](https://groups.google.com/forum/#!forum/exobrowser-dev)
- IRC Channel: #exobrowser on Freenode
- Twitter: @ExoBrowser
