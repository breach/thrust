// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

var DocumentNatives = requireNative('document_natives');

/******************************************************************************/
/* WEBVIEW ATTRIBUTES */
/******************************************************************************/
var WEB_VIEW_ATTRIBUTE_MAXHEIGHT = 'maxheight';
var WEB_VIEW_ATTRIBUTE_MAXWIDTH = 'maxwidth';
var WEB_VIEW_ATTRIBUTE_MINHEIGHT = 'minheight';
var WEB_VIEW_ATTRIBUTE_MINWIDTH = 'minwidth';

/** @type {Array.<string>} */
var WEB_VIEW_ATTRIBUTES = [
    'allowtransparency',
    'autosize',
    'name',
    'partition',
    WEB_VIEW_ATTRIBUTE_MINHEIGHT,
    WEB_VIEW_ATTRIBUTE_MINWIDTH,
    WEB_VIEW_ATTRIBUTE_MAXHEIGHT,
    WEB_VIEW_ATTRIBUTE_MAXWIDTH
];

/******************************************************************************/
/* WEBVIEWINTERNAL IMPLEMENTATION */
/******************************************************************************/
function WebViewInternal(webviewNode) {
  privates(webviewNode).internal = this;
  this.webviewNode = webviewNode;
  this.browserPluginNode = this.createBrowserPluginNode();
  var shadowRoot = this.webviewNode.createShadowRoot();
  shadowRoot.appendChild(this.browserPluginNode);
}

WebViewInternal.prototype.createBrowserPluginNode = function() {
  // We create BrowserPlugin as a custom element in order to observe changes
  // to attributes synchronously.
  var browserPluginNode = new WebViewInternal.BrowserPlugin();
  privates(browserPluginNode).internal = this;

  var ALL_ATTRIBUTES = WEB_VIEW_ATTRIBUTES.concat(['src']);
  $Array.forEach(ALL_ATTRIBUTES, function(attributeName) {
    // Only copy attributes that have been assigned values, rather than copying
    // a series of undefined attributes to BrowserPlugin.
    if (this.webviewNode.hasAttribute(attributeName)) {
      browserPluginNode.setAttribute(
        attributeName, this.webviewNode.getAttribute(attributeName));
    } else if (this.webviewNode[attributeName]){
      // Reading property using has/getAttribute does not work on
      // document.DOMContentLoaded event (but works on
      // window.DOMContentLoaded event).
      // So copy from property if copying from attribute fails.
      browserPluginNode.setAttribute(
        attributeName, this.webviewNode[attributeName]);
    }
  }, this);

  return browserPluginNode;
};

/**
 * @private
 */
WebViewInternal.prototype.setupFocusPropagation = function() {
  if (!this.webviewNode.hasAttribute('tabIndex')) {
    // <webview> needs a tabIndex in order to be focusable.
    // TODO(fsamuel): It would be nice to avoid exposing a tabIndex attribute
    // to allow <webview> to be focusable.
    // See http://crbug.com/231664.
    this.webviewNode.setAttribute('tabIndex', -1);
  }
  var self = this;
  this.webviewNode.addEventListener('focus', function(e) {
    // Focus the BrowserPlugin when the <webview> takes focus.
    self.browserPluginNode.focus();
  });
  this.webviewNode.addEventListener('blur', function(e) {
    // Blur the BrowserPlugin when the <webview> loses focus.
    self.browserPluginNode.blur();
  });
};

WebViewInternal.prototype.handleBrowserPluginAttributeMutation =
    function(name, newValue) {
  // This observer monitors mutations to attributes of the BrowserPlugin and
  // updates the <webview> attributes accordingly.
  // |newValue| is null if the attribute |name| has been removed.
  if (newValue != null) {
    // Update the <webview> attribute to match the BrowserPlugin attribute.
    // Note: Calling setAttribute on <webview> will trigger its mutation
    // observer which will then propagate that attribute to BrowserPlugin. In
    // cases where we permit assigning a BrowserPlugin attribute the same value
    // again (such as navigation when crashed), this could end up in an infinite
    // loop. Thus, we avoid this loop by only updating the <webview> attribute
    // if the BrowserPlugin attributes differs from it.
    if (newValue != this.webviewNode.getAttribute(name)) {
      this.webviewNode.setAttribute(name, newValue);
    }
  } else {
    // If an attribute is removed from the BrowserPlugin, then remove it
    // from the <webview> as well.
    this.webviewNode.removeAttribute(name);
  }
};

/******************************************************************************/
/* BROWSER-PLUGIN REGISTRATION */
/******************************************************************************/
function registerBrowserPluginElement() {
  var proto = Object.create(HTMLObjectElement.prototype);

  proto.createdCallback = function() {
    this.setAttribute('type', 'application/browser-plugin');
    // The <object> node fills in the <webview> container.
    this.style.width = '100%';
    this.style.height = '100%';
    console.log('BROWSER PLUGIN CREATED');
  };

  proto.attributeChangedCallback = function(name, oldValue, newValue) {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    internal.handleBrowserPluginAttributeMutation(name, newValue);
  };

  proto.attachedCallback = function() {
    // Load the plugin immediately.
    var unused = this.nonExistentAttribute;
  };

  console.log('BROWSER-PLUGIN REGISTERED');
  WebViewInternal.BrowserPlugin =
      DocumentNatives.RegisterElement('browser-plugin', {extends: 'object',
                                                         prototype: proto});
  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}

/******************************************************************************/
/* WEVIEW REGISTRATION */
/******************************************************************************/
function registerWebViewElement() {
  var proto = Object.create(HTMLElement.prototype);

  proto.createdCallback = function() {
    new WebViewInternal(this);
    console.log('WEBVIEW CREATED');
  };

  proto.test = function() {
    return 'TEST';
  };

  console.log('WEBVIEW REGISTERED');
  window.WebView =
      DocumentNatives.RegisterElement('webview', { prototype: proto });
  console.log(window.WebView);

  // Delete the callbacks so developers cannot call them and produce unexpected
  // behavior.
  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}

var useCapture = true;
window.addEventListener('readystatechange', function listener(event) {
  if (document.readyState == 'loading')
    return;

  registerBrowserPluginElement();
  registerWebViewElement();
  window.removeEventListener(event.type, listener, useCapture);
}, useCapture);

//exports.WebView = WebView;
exports.WebViewInternal = WebViewInternal;
