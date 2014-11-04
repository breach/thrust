// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This module implements Webview (<webview>) as a custom element that wraps a
// BrowserPlugin object element. The object element is hidden within
// the shadow DOM of the Webview element.

var DocumentNatives = requireNative('document_natives');
var GuestViewInternal =
    require('binding').Binding.create('guestViewInternal').generate();
var IdGenerator = requireNative('id_generator');
// TODO(lazyboy): Rename this to WebViewInternal and call WebViewInternal
// something else.
var WebView = require('webViewInternal').WebView;
var WebViewEvents = require('webViewEvents').WebViewEvents;

var WEB_VIEW_ATTRIBUTE_AUTOSIZE = 'autosize';
var WEB_VIEW_ATTRIBUTE_MAXHEIGHT = 'maxheight';
var WEB_VIEW_ATTRIBUTE_MAXWIDTH = 'maxwidth';
var WEB_VIEW_ATTRIBUTE_MINHEIGHT = 'minheight';
var WEB_VIEW_ATTRIBUTE_MINWIDTH = 'minwidth';
var AUTO_SIZE_ATTRIBUTES = [
  WEB_VIEW_ATTRIBUTE_AUTOSIZE,
  WEB_VIEW_ATTRIBUTE_MAXHEIGHT,
  WEB_VIEW_ATTRIBUTE_MAXWIDTH,
  WEB_VIEW_ATTRIBUTE_MINHEIGHT,
  WEB_VIEW_ATTRIBUTE_MINWIDTH
];

var WEB_VIEW_ATTRIBUTE_PARTITION = 'partition';

var PLUGIN_METHOD_ATTACH = '-internal-attach';

var ERROR_MSG_ALREADY_NAVIGATED =
    'The object has already navigated, so its partition cannot be changed.';
var ERROR_MSG_INVALID_PARTITION_ATTRIBUTE = 'Invalid partition attribute.';

/** @type {Array.<string>} */
var WEB_VIEW_ATTRIBUTES = [
    'allowtransparency',
];

/** @class representing state of storage partition. */
function Partition() {
  this.validPartitionId = true;
  this.persistStorage = false;
  this.storagePartitionId = '';
};

Partition.prototype.toAttribute = function() {
  if (!this.validPartitionId) {
    return '';
  }
  return (this.persistStorage ? 'persist:' : '') + this.storagePartitionId;
};

Partition.prototype.fromAttribute = function(value, hasNavigated) {
  var result = {};
  if (hasNavigated) {
    result.error = ERROR_MSG_ALREADY_NAVIGATED;
    return result;
  }
  if (!value) {
    value = '';
  }

  var LEN = 'persist:'.length;
  if (value.substr(0, LEN) == 'persist:') {
    value = value.substr(LEN);
    if (!value) {
      this.validPartitionId = false;
      result.error = ERROR_MSG_INVALID_PARTITION_ATTRIBUTE;
      return result;
    }
    this.persistStorage = true;
  } else {
    this.persistStorage = false;
  }

  this.storagePartitionId = value;
  return result;
};

// Implemented when the experimental API is available.
WebViewInternal.maybeRegisterExperimentalAPIs = function(proto) {}

/**
 * @constructor
 */
function WebViewInternal(webviewNode) {
  privates(webviewNode).internal = this;
  this.webviewNode = webviewNode;
  this.attached = false;
  this.elementAttached = false;

  this.beforeFirstNavigation = true;
  this.validPartitionId = true;
  // Used to save some state upon deferred attachment.
  // If <object> bindings is not available, we defer attachment.
  // This state contains whether or not the attachment request was for
  // newwindow.
  this.deferredAttachState = null;

  // on* Event handlers.
  this.on = {};

  this.browserPluginNode = this.createBrowserPluginNode();
  var shadowRoot = this.webviewNode.createShadowRoot();
  shadowRoot.appendChild(this.browserPluginNode);

  this.setupWebviewNodeAttributes();
  this.setupFocusPropagation();
  this.setupWebviewNodeProperties();

  this.viewInstanceId = IdGenerator.GetNextId();

  this.partition = new Partition();
  this.parseAttributes();

  new WebViewEvents(this, this.viewInstanceId);
}

/**
 * @private
 */
WebViewInternal.prototype.createBrowserPluginNode = function() {
  // We create BrowserPlugin as a custom element in order to observe changes
  // to attributes synchronously.
  var browserPluginNode = new WebViewInternal.BrowserPlugin();
  privates(browserPluginNode).internal = this;

  $Array.forEach(WEB_VIEW_ATTRIBUTES, function(attributeName) {
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

WebViewInternal.prototype.getInstanceId = function() {
  return this.instanceId;
};

/**
 * Resets some state upon reattaching <webview> element to the DOM.
 */
WebViewInternal.prototype.resetUponReattachment = function() {
  this.instanceId = undefined;
  this.beforeFirstNavigation = true;
  this.validPartitionId = true;
  this.partition.validPartitionId = true;
};

// Sets <webview>.request property.
WebViewInternal.prototype.setRequestPropertyOnWebViewNode = function(request) {
  Object.defineProperty(
      this.webviewNode,
      'request',
      {
        value: request,
        enumerable: true
      }
  );
};

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

/**
 * @private
 */
WebViewInternal.prototype.back = function() {
  return this.go(-1);
};

/**
 * @private
 */
WebViewInternal.prototype.forward = function() {
  return this.go(1);
};

/**
 * @private
 */
WebViewInternal.prototype.canGoBack = function() {
  return this.entryCount > 1 && this.currentEntryIndex > 0;
};

/**
 * @private
 */
WebViewInternal.prototype.canGoForward = function() {
  return this.currentEntryIndex >= 0 &&
      this.currentEntryIndex < (this.entryCount - 1);
};

/**
 * @private
 */
WebViewInternal.prototype.clearData = function() {
  if (!this.instanceId) {
    return;
  }
  var args = $Array.concat([this.instanceId], $Array.slice(arguments));
  $Function.apply(WebView.clearData, null, args);
};

/**
 * @private
 */
WebViewInternal.prototype.getProcessId = function() {
  return this.processId;
};

/**
 * @private
 */
WebViewInternal.prototype.go = function(relativeIndex) {
  if (!this.instanceId) {
    return;
  }
  WebView.go(this.instanceId, relativeIndex);
};

/**
 * @private
 */
WebViewInternal.prototype.print = function() {
  this.executeScript({code: 'window.print();'});
};

/**
 * @private
 */
WebViewInternal.prototype.reload = function() {
  if (!this.instanceId) {
    return;
  }
  WebView.reload(this.instanceId);
};

/**
 * @private
 */
WebViewInternal.prototype.stop = function() {
  if (!this.instanceId) {
    return;
  }
  WebView.stop(this.instanceId);
};

/**
 * @private
 */
WebViewInternal.prototype.terminate = function() {
  if (!this.instanceId) {
    return;
  }
  WebView.terminate(this.instanceId);
};

/**
 * @private
 */
WebViewInternal.prototype.validateExecuteCodeCall  = function() {
  var ERROR_MSG_CANNOT_INJECT_SCRIPT = '<webview>: ' +
      'Script cannot be injected into content until the page has loaded.';
  if (!this.instanceId) {
    throw new Error(ERROR_MSG_CANNOT_INJECT_SCRIPT);
  }
};

/**
 * @private
 */
WebViewInternal.prototype.executeScript = function(var_args) {
  this.validateExecuteCodeCall();
  var args = $Array.concat([this.instanceId, this.src],
                           $Array.slice(arguments));
  $Function.apply(WebView.executeScript, null, args);
};

/**
 * @private
 */
WebViewInternal.prototype.insertCSS = function(var_args) {
  this.validateExecuteCodeCall();
  var args = $Array.concat([this.instanceId, this.src],
                           $Array.slice(arguments));
  $Function.apply(WebView.insertCSS, null, args);
};

WebViewInternal.prototype.setupAutoSizeProperties = function() {
  var self = this;
  $Array.forEach(AUTO_SIZE_ATTRIBUTES, function(attributeName) {
    this[attributeName] = this.webviewNode.getAttribute(attributeName);
    Object.defineProperty(this.webviewNode, attributeName, {
      get: function() {
        return self[attributeName];
      },
      set: function(value) {
        self.webviewNode.setAttribute(attributeName, value);
      },
      enumerable: true
    });
  }, this);
};

/**
 * @private
 */
WebViewInternal.prototype.setupWebviewNodeProperties = function() {
  var ERROR_MSG_CONTENTWINDOW_NOT_AVAILABLE = '<webview>: ' +
    'contentWindow is not available at this time. It will become available ' +
        'when the page has finished loading.';

  this.setupAutoSizeProperties();
  var self = this;
  var browserPluginNode = this.browserPluginNode;
  // Expose getters and setters for the attributes.
  $Array.forEach(WEB_VIEW_ATTRIBUTES, function(attributeName) {
    Object.defineProperty(this.webviewNode, attributeName, {
      get: function() {
        if (browserPluginNode.hasOwnProperty(attributeName)) {
          return browserPluginNode[attributeName];
        } else {
          return browserPluginNode.getAttribute(attributeName);
        }
      },
      set: function(value) {
        if (browserPluginNode.hasOwnProperty(attributeName)) {
          // Give the BrowserPlugin first stab at the attribute so that it can
          // throw an exception if there is a problem. This attribute will then
          // be propagated back to the <webview>.
          browserPluginNode[attributeName] = value;
        } else {
          browserPluginNode.setAttribute(attributeName, value);
        }
      },
      enumerable: true
    });
  }, this);

  // <webview> src does not quite behave the same as BrowserPlugin src, and so
  // we don't simply keep the two in sync.
  this.src = this.webviewNode.getAttribute('src');
  Object.defineProperty(this.webviewNode, 'src', {
    get: function() {
      return self.src;
    },
    set: function(value) {
      self.webviewNode.setAttribute('src', value);
    },
    // No setter.
    enumerable: true
  });

  Object.defineProperty(this.webviewNode, 'name', {
    get: function() {
      return self.name;
    },
    set: function(value) {
      self.webviewNode.setAttribute('name', value);
    },
    enumerable: true
  });

  Object.defineProperty(this.webviewNode, 'partition', {
    get: function() {
      return self.partition.toAttribute();
    },
    set: function(value) {
      var result = self.partition.fromAttribute(value, self.hasNavigated());
      if (result.error) {
        throw result.error;
      }
      self.webviewNode.setAttribute('partition', value);
    },
    enumerable: true
  });

  // We cannot use {writable: true} property descriptor because we want a
  // dynamic getter value.
  Object.defineProperty(this.webviewNode, 'contentWindow', {
    get: function() {
      if (browserPluginNode.contentWindow)
        return browserPluginNode.contentWindow;
      window.console.error(ERROR_MSG_CONTENTWINDOW_NOT_AVAILABLE);
    },
    // No setter.
    enumerable: true
  });
};

/**
 * @private
 */
WebViewInternal.prototype.setupWebviewNodeAttributes = function() {
  this.setupWebViewSrcAttributeMutationObserver();
};

/**
 * @private
 */
WebViewInternal.prototype.setupWebViewSrcAttributeMutationObserver =
    function() {
  // The purpose of this mutation observer is to catch assignment to the src
  // attribute without any changes to its value. This is useful in the case
  // where the webview guest has crashed and navigating to the same address
  // spawns off a new process.
  var self = this;
  this.srcAndPartitionObserver = new MutationObserver(function(mutations) {
    $Array.forEach(mutations, function(mutation) {
      var oldValue = mutation.oldValue;
      var newValue = self.webviewNode.getAttribute(mutation.attributeName);
      if (oldValue != newValue) {
        return;
      }
      self.handleWebviewAttributeMutation(
          mutation.attributeName, oldValue, newValue);
    });
  });
  var params = {
    attributes: true,
    attributeOldValue: true,
    attributeFilter: ['src', 'partition']
  };
  this.srcAndPartitionObserver.observe(this.webviewNode, params);
};

/**
 * @private
 */
WebViewInternal.prototype.handleWebviewAttributeMutation =
      function(name, oldValue, newValue) {
  // This observer monitors mutations to attributes of the <webview> and
  // updates the BrowserPlugin properties accordingly. In turn, updating
  // a BrowserPlugin property will update the corresponding BrowserPlugin
  // attribute, if necessary. See BrowserPlugin::UpdateDOMAttribute for more
  // details.
  if (AUTO_SIZE_ATTRIBUTES.indexOf(name) > -1) {
    this[name] = newValue;
    if (!this.instanceId) {
      return;
    }
    // Convert autosize attribute to boolean.
    var autosize = this.webviewNode.hasAttribute(WEB_VIEW_ATTRIBUTE_AUTOSIZE);
    GuestViewInternal.setAutoSize(this.instanceId, {
      'enableAutoSize': autosize,
      'min': {
        'width': parseInt(this.minwidth || 0),
        'height': parseInt(this.minheight || 0)
      },
      'max': {
        'width': parseInt(this.maxwidth || 0),
        'height': parseInt(this.maxheight || 0)
      }
    });
    return;
  } else if (name == 'name') {
    // We treat null attribute (attribute removed) and the empty string as
    // one case.
    oldValue = oldValue || '';
    newValue = newValue || '';

    if (oldValue === newValue) {
      return;
    }
    this.name = newValue;
    if (!this.instanceId) {
      return;
    }
    WebView.setName(this.instanceId, newValue);
    return;
  } else if (name == 'src') {
    // We treat null attribute (attribute removed) and the empty string as
    // one case.
    oldValue = oldValue || '';
    newValue = newValue || '';
    // Once we have navigated, we don't allow clearing the src attribute.
    // Once <webview> enters a navigated state, it cannot be return back to a
    // placeholder state.
    if (newValue == '' && oldValue != '') {
      // src attribute changes normally initiate a navigation. We suppress
      // the next src attribute handler call to avoid reloading the page
      // on every guest-initiated navigation.
      this.ignoreNextSrcAttributeChange = true;
      this.webviewNode.setAttribute('src', oldValue);
      return;
    }
    this.src = newValue;
    if (this.ignoreNextSrcAttributeChange) {
      // Don't allow the src mutation observer to see this change.
      this.srcAndPartitionObserver.takeRecords();
      this.ignoreNextSrcAttributeChange = false;
      return;
    }
    var result = {};
    this.parseSrcAttribute(result);

    if (result.error) {
      throw result.error;
    }
  } else if (name == 'partition') {
    // Note that throwing error here won't synchronously propagate.
    this.partition.fromAttribute(newValue, this.hasNavigated());
  }

  // No <webview> -> <object> mutation propagation for these attributes.
  if (name == 'src' || name == 'partition') {
    return;
  }

  if (this.browserPluginNode.hasOwnProperty(name)) {
    this.browserPluginNode[name] = newValue;
  } else {
    this.browserPluginNode.setAttribute(name, newValue);
  }
};

/**
 * @private
 */
WebViewInternal.prototype.handleBrowserPluginAttributeMutation =
    function(name, oldValue, newValue) {
  if (name == 'internalbindings' && !oldValue && newValue) {
    this.browserPluginNode.removeAttribute('internalbindings');

    if (this.deferredAttachState) {
      var self = this;
      // A setTimeout is necessary for the binding to be initialized properly.
      window.setTimeout(function() {
        if (self.hasBindings()) {
          var params = self.buildAttachParams(
              self.deferredAttachState.isNewWindow);
          self.browserPluginNode[PLUGIN_METHOD_ATTACH](self.instanceId, params);
          self.deferredAttachState = null;
        }
      }, 0);
    }
    return;
  }

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

WebViewInternal.prototype.onSizeChanged = function(webViewEvent) {
  var newWidth = webViewEvent.newWidth;
  var newHeight = webViewEvent.newHeight;

  var node = this.webviewNode;

  var width = node.offsetWidth;
  var height = node.offsetHeight;

  // Check the current bounds to make sure we do not resize <webview>
  // outside of current constraints.
  var maxWidth;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MAXWIDTH) &&
      node[WEB_VIEW_ATTRIBUTE_MAXWIDTH]) {
    maxWidth = node[WEB_VIEW_ATTRIBUTE_MAXWIDTH];
  } else {
    maxWidth = width;
  }

  var minWidth;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MINWIDTH) &&
      node[WEB_VIEW_ATTRIBUTE_MINWIDTH]) {
    minWidth = node[WEB_VIEW_ATTRIBUTE_MINWIDTH];
  } else {
    minWidth = width;
  }
  if (minWidth > maxWidth) {
    minWidth = maxWidth;
  }

  var maxHeight;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MAXHEIGHT) &&
      node[WEB_VIEW_ATTRIBUTE_MAXHEIGHT]) {
    maxHeight = node[WEB_VIEW_ATTRIBUTE_MAXHEIGHT];
  } else {
    maxHeight = height;
  }
  var minHeight;
  if (node.hasAttribute(WEB_VIEW_ATTRIBUTE_MINHEIGHT) &&
      node[WEB_VIEW_ATTRIBUTE_MINHEIGHT]) {
    minHeight = node[WEB_VIEW_ATTRIBUTE_MINHEIGHT];
  } else {
    minHeight = height;
  }
  if (minHeight > maxHeight) {
    minHeight = maxHeight;
  }

  if (!this.webviewNode.hasAttribute(WEB_VIEW_ATTRIBUTE_AUTOSIZE) ||
      (newWidth >= minWidth &&
       newWidth <= maxWidth &&
       newHeight >= minHeight &&
       newHeight <= maxHeight)) {
    node.style.width = newWidth + 'px';
    node.style.height = newHeight + 'px';
    // Only fire the DOM event if the size of the <webview> has actually
    // changed.
    this.dispatchEvent(webViewEvent);
  }
};

// Returns true if Browser Plugin bindings is available.
// Bindings are unavailable if <object> is not in the render tree.
WebViewInternal.prototype.hasBindings = function() {
  return 'function' == typeof this.browserPluginNode[PLUGIN_METHOD_ATTACH];
};

WebViewInternal.prototype.hasNavigated = function() {
  return !this.beforeFirstNavigation;
};

/** @return {boolean} */
WebViewInternal.prototype.parseSrcAttribute = function(result) {
  if (!this.partition.validPartitionId) {
    result.error = ERROR_MSG_INVALID_PARTITION_ATTRIBUTE;
    return false;
  }
  this.src = this.webviewNode.getAttribute('src');

  if (!this.src) {
    return true;
  }

  if (!this.elementAttached) {
    return true;
  }

  if (!this.hasGuestInstanceID()) {
    if (this.beforeFirstNavigation) {
      this.beforeFirstNavigation = false;
      this.allocateInstanceId();
    }
    return true;
  }

  // Navigate to this.src.
  WebView.navigate(this.instanceId, this.src);
  return true;
};

/** @return {boolean} */
WebViewInternal.prototype.parseAttributes = function() {
  var hasNavigated = this.hasNavigated();
  var attributeValue = this.webviewNode.getAttribute('partition');
  var result = this.partition.fromAttribute(attributeValue, hasNavigated);
  return this.parseSrcAttribute(result);
};

WebViewInternal.prototype.hasGuestInstanceID = function() {
  return this.instanceId != undefined;
};

WebViewInternal.prototype.allocateInstanceId = function() {
  var storagePartitionId =
      this.webviewNode.getAttribute(WEB_VIEW_ATTRIBUTE_PARTITION) ||
      this.webviewNode[WEB_VIEW_ATTRIBUTE_PARTITION];
  var params = {
    'storagePartitionId': storagePartitionId,
  };
  var self = this;
  GuestViewInternal.createGuest(
      'webview',
      params,
      function(instanceId) {
        // TODO(lazyboy): Make sure this.autoNavigate_ stuff correctly updated
        // |self.src| at this point.
        self.attachWindow(instanceId, false);
      });
};

WebViewInternal.prototype.onFrameNameChanged = function(name) {
  this.name = name || '';
  if (this.name === '') {
    this.webviewNode.removeAttribute('name');
  } else {
    this.webviewNode.setAttribute('name', this.name);
  }
};

WebViewInternal.prototype.dispatchEvent = function(webViewEvent) {
  return this.webviewNode.dispatchEvent(webViewEvent);
};

/**
 * Adds an 'on<event>' property on the webview, which can be used to set/unset
 * an event handler.
 */
WebViewInternal.prototype.setupEventProperty = function(eventName) {
  var propertyName = 'on' + eventName.toLowerCase();
  var self = this;
  var webviewNode = this.webviewNode;
  Object.defineProperty(webviewNode, propertyName, {
    get: function() {
      return self.on[propertyName];
    },
    set: function(value) {
      if (self.on[propertyName])
        webviewNode.removeEventListener(eventName, self.on[propertyName]);
      self.on[propertyName] = value;
      if (value)
        webviewNode.addEventListener(eventName, value);
    },
    enumerable: true
  });
};

// Updates state upon loadcommit.
WebViewInternal.prototype.onLoadCommit = function(
    currentEntryIndex, entryCount, processId, url, isTopLevel) {
  this.currentEntryIndex = currentEntryIndex;
  this.entryCount = entryCount;
  this.processId = processId;
  var oldValue = this.webviewNode.getAttribute('src');
  var newValue = url;
  if (isTopLevel && (oldValue != newValue)) {
    // Touching the src attribute triggers a navigation. To avoid
    // triggering a page reload on every guest-initiated navigation,
    // we use the flag ignoreNextSrcAttributeChange here.
    this.ignoreNextSrcAttributeChange = true;
    this.webviewNode.setAttribute('src', newValue);
  }
};

WebViewInternal.prototype.onAttach = function(storagePartitionId) {
  this.webviewNode.setAttribute('partition', storagePartitionId);
  this.partition.fromAttribute(storagePartitionId, this.hasNavigated());
};


/** @private */
WebViewInternal.prototype.getUserAgent = function() {
  return this.userAgentOverride || navigator.userAgent;
};

/** @private */
WebViewInternal.prototype.isUserAgentOverridden = function() {
  return !!this.userAgentOverride &&
      this.userAgentOverride != navigator.userAgent;
};

/** @private */
WebViewInternal.prototype.setUserAgentOverride = function(userAgentOverride) {
  this.userAgentOverride = userAgentOverride;
  if (!this.instanceId) {
    // If we are not attached yet, then we will pick up the user agent on
    // attachment.
    return;
  }
  WebView.overrideUserAgent(this.instanceId, userAgentOverride);
};

/** @private */
WebViewInternal.prototype.find = function(search_text, options, callback) {
  if (!this.instanceId) {
    return;
  }
  WebView.find(this.instanceId, search_text, options, callback);
};

/** @private */
WebViewInternal.prototype.stopFinding = function(action) {
  if (!this.instanceId) {
    return;
  }
  WebView.stopFinding(this.instanceId, action);
};

/** @private */
WebViewInternal.prototype.setZoom = function(zoomFactor, callback) {
  if (!this.instanceId) {
    return;
  }
  WebView.setZoom(this.instanceId, zoomFactor, callback);
};

WebViewInternal.prototype.getZoom = function(callback) {
  if (!this.instanceId) {
    return;
  }
  WebView.getZoom(this.instanceId, callback);
};

WebViewInternal.prototype.buildAttachParams = function(isNewWindow) {
  var params = {
    'autosize': this.webviewNode.hasAttribute(WEB_VIEW_ATTRIBUTE_AUTOSIZE),
    'instanceId': this.viewInstanceId,
    'maxheight': parseInt(this.maxheight || 0),
    'maxwidth': parseInt(this.maxwidth || 0),
    'minheight': parseInt(this.minheight || 0),
    'minwidth': parseInt(this.minwidth || 0),
    'name': this.name,
    // We don't need to navigate new window from here.
    'src': isNewWindow ? undefined : this.src,
    // If we have a partition from the opener, that will also be already
    // set via this.onAttach().
    'storagePartitionId': this.partition.toAttribute(),
    'userAgentOverride': this.userAgentOverride
  };
  return params;
};

WebViewInternal.prototype.attachWindow = function(instanceId, isNewWindow) {
  this.instanceId = instanceId;
  var params = this.buildAttachParams(isNewWindow);

  if (!this.hasBindings()) {
    // No bindings means that the plugin isn't there (display: none), we defer
    // attachWindow in this case.
    this.deferredAttachState = {isNewWindow: isNewWindow};
    return false;
  }

  this.deferredAttachState = null;
  return this.browserPluginNode[PLUGIN_METHOD_ATTACH](this.instanceId, params);
};

// Registers browser plugin <object> custom element.
function registerBrowserPluginElement() {
  var proto = Object.create(HTMLObjectElement.prototype);

  proto.createdCallback = function() {
    this.setAttribute('type', 'application/browser-plugin');
    // The <object> node fills in the <webview> container.
    this.style.width = '100%';
    this.style.height = '100%';
  };

  proto.attributeChangedCallback = function(name, oldValue, newValue) {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    internal.handleBrowserPluginAttributeMutation(name, oldValue, newValue);
  };

  proto.attachedCallback = function() {
    // Load the plugin immediately.
    var unused = this.nonExistentAttribute;
  };

  WebViewInternal.BrowserPlugin =
      DocumentNatives.RegisterElement('browserplugin', {extends: 'object',
                                                        prototype: proto});

  delete proto.createdCallback;
  delete proto.attachedCallback;
  delete proto.detachedCallback;
  delete proto.attributeChangedCallback;
}

// Registers <webview> custom element.
function registerWebViewElement() {
  var proto = Object.create(HTMLElement.prototype);

  proto.createdCallback = function() {
    new WebViewInternal(this);
  };

  proto.attributeChangedCallback = function(name, oldValue, newValue) {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    internal.handleWebviewAttributeMutation(name, oldValue, newValue);
  };

  proto.detachedCallback = function() {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    internal.elementAttached = false;
  };

  proto.attachedCallback = function() {
    var internal = privates(this).internal;
    if (!internal) {
      return;
    }
    if (!internal.elementAttached) {
      internal.elementAttached = true;
      internal.resetUponReattachment();
      internal.parseAttributes();
    }
  };

  var methods = [
    'back',
    'find',
    'forward',
    'canGoBack',
    'canGoForward',
    'clearData',
    'getProcessId',
    'getZoom',
    'go',
    'print',
    'reload',
    'setZoom',
    'stop',
    'stopFinding',
    'terminate',
    'executeScript',
    'insertCSS',
    'getUserAgent',
    'isUserAgentOverridden',
    'setUserAgentOverride'
  ];

  // Forward proto.foo* method calls to WebViewInternal.foo*.
  for (var i = 0; methods[i]; ++i) {
    var createHandler = function(m) {
      return function(var_args) {
        var internal = privates(this).internal;
        return $Function.apply(internal[m], internal, arguments);
      };
    };
    proto[methods[i]] = createHandler(methods[i]);
  }

  WebViewInternal.maybeRegisterExperimentalAPIs(proto);

  window.WebView =
      DocumentNatives.RegisterElement('webview', {prototype: proto});

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

/**
 * Implemented when the experimental API is available.
 * @private
 */
WebViewInternal.prototype.maybeGetExperimentalEvents = function() {};

/**
 * Implemented when the experimental API is available.
 * @private
 */
WebViewInternal.prototype.maybeGetExperimentalPermissions = function() {
  return [];
};

/**
 * Calls to show contextmenu right away instead of dispatching a 'contextmenu'
 * event.
 * This will be overridden in web_view_experimental.js to implement contextmenu
 * API.
 */
WebViewInternal.prototype.maybeHandleContextMenu = function(e, webViewEvent) {
  var requestId = e.requestId;
  // Setting |params| = undefined will show the context menu unmodified, hence
  // the 'contextmenu' API is disabled for stable channel.
  var params = undefined;
  WebView.showContextMenu(this.instanceId, requestId, params);
};

/**
 * Implemented when the experimental API is available.
 * @private
 */
WebViewInternal.prototype.setupExperimentalContextMenus = function() {};

exports.WebView = WebView;
exports.WebViewInternal = WebViewInternal;
