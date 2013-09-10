---
layout: default
title: "ExoBrowser"
---


The ExoBrowser is a **scriptable platform** designed to ease the experimentation with new concepts for the Web Browser.

The ExoBrowser exposes its API (parts of the Chromium Content API + a Simple View Model) in Javascript and enables the implementation of a fully functional browser entirely out of it (as a Javascript/HTML/CSS app)

See [The Experimentation Platform to Build a Next Generation Browser](https://github.com/spolu/exo_browser/wiki/The-Experimentation-Platform-to-Build-a-Next-generation-Web-Browser)

### Exp.1: Stacked Navigation

Today we release a **first experiment** towards a new Browser. This release experiments with the *Stacked Navigation*:

```
- Vertical list of Tabs / LRU Ordered
- Pin / Unpin Tabs at the top of the Stack
- Search in opened Tabs
```

Based on the ExoBrowser platform, it is entirely written in Javascript.

[Quick Intro to the Experiment (slides)](http://slid.es/stanislaspolu/exo_browser-exp1)

### Downloads

- *OSX [ia32]* [exo_browser-v0.1.exp1-osx-ia32.zip](http://bit.ly/1ec5MYP) (122,0 MB)
- *Linux [x64]* [exo_browser-v0.1.exp1-linux-x64.tar.gz](http://bit.ly/15g43Mv) (32,7 MB)

### Hacking with the ExoBrowser

You can **dump** the Javascript code of the experiment locally, **modify** it, and **interpret** it directly with the ExoBrowser executable.

*On Mac OS X*:
{% highlight bash %}
$ ./ExoBrowser.app/Contents/MacOS/ExoBrowser --dump-app
$ cd app/
...
$ open ExoBrowser.app --args --raw app/index.js
{% endhighlight %}

*On Linux*:
{% highlight bash %}
$ ./exo_browser --dump-app
$ cd app/
...
$ ./exo_browser --raw app/index.js
{% endhighlight %}

See the [ExoBrowser JS API Specification](https://github.com/spolu/exo_browser/blob/master/API.md)

### Get in Touch

- Github: [https://github.com/spolu/exo_browser](https://github.com/spolu/exo_browser)
- Mailing list: [exobrowser-dev@googlegroups.com](https://groups.google.com/forum/#!forum/exobrowser-dev)
- IRC Channel: #exobrowser on Freenode
- Twitter: [@ExoBrowser](https://twitter.com/ExoBrowser)

