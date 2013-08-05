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
/* EXOBROWSER API */

/* EXOBROWSER */
_breach.createNewExoBrowser({
  size: [123, 23]
}, cb_);
b.size(cb_);
b.position(cb_);
b.kill(cb_);

b.setOpenURLCallback(cb_);
b.setResizeCallback(cb_);
b.setAddWebContentsCallback(cb_);

/* EXOBROWSER FRAME */
b.addFrame({
  name: '',
  url: '',
  visible: true,
  position: [0, 24],
  size: [640, 418]
  zIndex: 0
}, cb_);

f.setVisible(true, cb_);
f.setPosition([0, 23], cb_);
f.setSize([120, 230], cb_);
f.setZIndex(100, cb_);

f.size(cb_);
f.position(cb_);

f.loadURL(url, cb_);
f.goBack(cb_);
f.goFoward(cb_);
f.reload(cb_);
f.stop(cb_);

f.name(cb_);
f.parent(cb_); 

b.killFrame(name, cb_);


/* EXOBROWSER NETWORKING */ 

```

