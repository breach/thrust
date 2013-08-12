Breach
======

### API Specification

The API is enterely async as actual commands are run on the `BrowserThread::UI`
so that the nodeJS thread do not block on possible lenghty UI operations.

The ExoBrowser handles a list of ExoFrame which are basically tabs. Only one is
visible at a time and they display as big as possible. Additionally, 

No DevTools ATM

Native objects live on the BrowserThread::UI thread but are aware of their
JS counterparts Wrappers.

#### API v0.alpha:

```
var b = _breach.exo_browser({
  size: [123, 23]
});

exo_browser.ready();
exo_browser#ready()

exo_browser#open_url(url)
exo_browser#resize()
exo_browser#kill()

?exo_browser#frame_created(frame)
exo_browser#frame_loading_state_change(frame, loading)
exo_browser#frame_close(frame)
exo_browser#frame_navigate(frame, url)
exo_browser#frame_title_updated(frame, title)

exo_browser.frame(name);

exo_browser.frames();
exo_browser.controls();
exo_browser.pages();

exo_browser.set_control(type, frame, [cb_]);
exo_browser.unset_control(type, [cb_]);
exo_browser.set_control_dimension(type, size, [cb_]);

exo_browser.add_page(frame, [cb_]);
exo_browser.remove_page(frame, [cb_]);
exo_browser.show_page(frame, [cb_]);


var f = _breach.exo_frame({
  name: '',
  url: '',
});

exo_frame.ready();
exo_frame#ready();

exo_frame.url();
exo_frame.name();
exo_frame.visible();
exo_frame.ready();
exo_frame.parent();
exo_frame.type();
exo_frame.loading();
exo_frame.title();

exo_frame.load_url(url, [cb_]);
exo_frame.go_back_or_forward(offset, [cb_]);
exo_frame.reload([cb_]);
exo_frame.stop([cb_]);
```


#### Internal API v0.alpha:

```
/* EXOBROWSER */
_breach._createExoBrowser({
  size: [123, 23]
}, cb_);

b.size(cb_);
b.position(cb_);

b._setOpenURLCallback(cb_);
b._setResizeCallback(cb_);
b._setKillCallback(cb_);

b._setFrameLoadingStateChangeCallback(cb_);
b._setFrameCloseCallback(cb_);
b._setFrameNavigateCallback(cb_);
b._setFrameCreatedCallback(cb_);

b.kill(cb_);

/* EXOFRAME */
_breach._createExoFrame({
  name: '',
  url: '',
}, cb_);

f._loadURL(url, cb_);
f._goBackOrForward(offset, cb_);
f._reload(cb_);
f._stop(cb_);

f._name(cb_);
f._type(cb_);

/* CONTROL */

b._setControl(type, frame, cb_);
b._unsetControl(type, cb_);
b._setControlDimension(type, size, cb_);

/* PAGE */

b._addPage(frame, cb_);
b._removePage(name, cb_);
b._showPage(name, cb_);

/* EXOBROWSER NETWORKING */ 

```

### Getting the Source Code & Building Breach

Please refer to the Wiki Page: 
[Building Breach](https://github.com/spolu/breach/wiki/Building-Breach)

