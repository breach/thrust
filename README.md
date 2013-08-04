breach
======

### UI Specification

- WebViews: List of full screen webviews. One visible.
- Controls: List of webviews, (on top of tabs)

API v0:

```
/* BROWSER */ 

var b = new _breach.Browser();
b.close();
b.showDevTools();
b.closeDevTools();
b.size();
b.position();

/* BROWSER CONTROLS */

b.newControl(name, url, visible, position, size);
b.setControlVisible(name, visibility);
b.setControlPosition(name, size);
b.setControlSize(name, size);
b.killControl(name);

b.controls() = [{
  name: 'xxx',
  url: 'xxx',
  visible: false,
  position: [ 123, 231 ],
  size: [ 32, 213 ]
}, ...];
b.control(name) = {
  name: 'xxx',
  url: 'yyy',
  visible: false,
  position: [ 123, 231 ],
  size: [ 32, 213 ]
};

/* BROWSER WEBVIEWS */

b.webViews() = [{
  url: 'xxx,
  visible: true
}]
b.newWebView(url, visible);
b.showWebView(i);
b.killWebView(i);
b.on('web_view_change', function(webViews) {})

/* BROWSER NETWORKING */ 

```

