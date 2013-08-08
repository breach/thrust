breach
======

### API Specification

The API is enterely async as actual commands are run on the `BrowserThread::UI`
so that the nodeJS thread do not block on possible lenghty UI operations.

The ExoBrowser handles a list of ExoFrame which are basically tabs. Only one is
visible at a time and they display as big as possible. Additionally, 

No DevTools ATM

Native objects live on the BrowserThread::UI thread but are aware of their
JS counterparts Wrappers.


API v0:

```
/* EXOBROWSER LOWLEVEL API */

/* EXOBROWSER */
_breach._createNewExoBrowser({
  size: [123, 23]
}, cb_);
b.size(cb_);
b.position(cb_);
b.kill(cb_);

b._setOpenURLCallback(cb_);
b._setResizeCallback(cb_);
b._setBrowserCloseCallback(cb_);
b._setBrowserNewCallback(cb_);
b._setFrameLoadingStateChangeCallback(cb_);
b._setFrameCloseCallback(cb_);
b._setFrameNavigateCallback(cb_);
b._setFrameCreatedCallback(cb_);


/* EXOFRAME */
_breach._createNewExoFrame({
  name: '',
  url: '',
}, cb_);

f._size(cb_);
f._position(cb_);

?f._isVisible(cb_);

f._loadURL(url, cb_);
f._goBackOrForward(offset, cb_);
f._reload(cb_);
f._stop(cb_);

f._name(cb_);
f._type(cb_);

?f._parent(cb_); 


/* CONTROL */

b._setControl(type, frame, cb_);
b._unsetControl(type, cb_);
b._setControlDimension(type, pixles, cb_);

/* PAGE */

b._addPage(frame, cb_);
b._removePage(frame, cb_);
b._showPage(name, cb_);


/* EXOBROWSER NETWORKING */ 

```

### Getting the Source Code & Building Breach

Please refer to the Wiki Page: 
[Building Breach](https://github.com/spolu/breach/wiki/Building-Breach)

