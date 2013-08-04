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
var b = new _breach._ExoBrowser({
  size: [123, 23],
  position: [323, 232]
});
b.close(cb_);
b.getSize(cb_);
b.getPosition(cb_);
b.on('resize', function() {});

b.showDevTools(cb_);
b.closeDevTools(cb_);

/* EXOBROWSER FRAME */
var f = new _breach._Frame({
  name: '',
  url: '',
  visible: true,
  position: [0, 24],
  size: [640, 418]
  zIndex: 0
});
f.setVisible(true, cb_);
f.setPosition([0, 23], cb_);
f.setSize([120, 230], cb_);
f.setZIndex(100, cb_);
f.goToUrl(url, cb_);
f.next(cb_);
f.previous(cb_);

b.addFrame(v, cb_);
b.removeFrame(v, cb_);
b.getFrames(cb_);


/* EXOBROWSER NETWORKING */ 

```

