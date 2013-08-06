breach
======

### API Specification

The API is enterely async as actual commands are run on the `BrowserThread::UI`
so that the nodeJS thread do not block on possible lenghty UI operations.

The layout used to display the views is similar to a  `fixed` layout with 
z-Index and visibility properties.

Tests have to be performed to check that layouting on resize is fast enough.
DevTools are hosted on a separate window.

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
b._setAddFrameCallback(cb_);

/* EXOBROWSER FRAME */
_breach._createNewExoFrame({
  name: '',
  url: '',
  visible: true,
  position: [0, 24],
  size: [640, 418]
  zIndex: 0
}, cb_);

b._addFrame(f);
b._removeFrame(f);

f._setVisible(true, cb_);
f._setPosition([0, 23], cb_);
f._setSize([120, 230], cb_);
f._setZIndex(100, cb_);

f._size(cb_);
f._position(cb_);

f._loadURL(url, cb_);
f._goBack(cb_);
f._goFoward(cb_);
f._reload(cb_);
f._stop(cb_);

f._name(cb_);
f._parent(cb_); 

/* EXOBROWSER NETWORKING */ 

```

