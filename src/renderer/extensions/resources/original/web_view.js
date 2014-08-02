// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This module implements Webview (<webview>) as a custom element that wraps a
// BrowserPlugin object element. The object element is hidden within
// the shadow DOM of the Webview element.

var DocumentNatives = requireNative('document_natives');
var EventBindings = require('event_bindings');
var IdGenerator = requireNative('id_generator');
var MessagingNatives = requireNative('messaging_natives');
var WebRequestEvent = require('webRequestInternal').WebRequestEvent;
var WebRequestSchema =
    requireNative('schema_registry').GetSchema('webRequest');
var DeclarativeWebRequestSchema =
    requireNative('schema_registry').GetSchema('declarativeWebRequest');
var WebView = require('webview').WebView;

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

var CreateEvent = function(name) {
  var eventOpts = {supportsListeners: true, supportsFilters: true};
  return new EventBindings.Event(name, undefined, eventOpts);
};

// WEB_VIEW_EVENTS is a map of stable <webview> DOM event names to their
//     associated extension event descriptor objects.
// An event listener will be attached to the extension event |evt| specified in
//     the descriptor.
// |fields| specifies the public-facing fields in the DOM event that are
//     accessible to <webview> developers.
// |customHandler| allows a handler function to be called each time an extension
//     event is caught by its event listener. The DOM event should be dispatched
//     within this handler function. With no handler function, the DOM event
//     will be dispatched by default each time the extension event is caught.
// |cancelable| (default: false) specifies whether the event's default
//     behavior can be canceled. If the default action associated with the event
//     is prevented, then its dispatch function will return false in its event
//     handler. The event must have a custom handler for this to be meaningful.
var WEB_VIEW_EVENTS = {
  'close': {
    evt: CreateEvent('webview.onClose'),
    fields: []
  },
  'consolemessage': {
    evt: CreateEvent('webview.onConsoleMessage'),
    fields: ['level', 'message', 'line', 'sourceId']
  },
  'contentload': {
    evt: CreateEvent('webview.onContentLoad'),
    fields: []
  },
  'dialog': {
    cancelable: true,
    customHandler: function(webViewInternal, event, webViewEvent) {
      webViewInternal.handleDialogEvent(event, webViewEvent);
    },
    evt: CreateEvent('webview.onDialog'),
    fields: ['defaultPromptText', 'messageText', 'messageType', 'url']
  },
  'exit': {
     evt: CreateEvent('webview.onExit'),
     fields: ['processId', 'reason']
  },
  'loadabort': {
    cancelable: true,
    customHandler: function(webViewInternal, event, webViewEvent) {
      webViewInternal.handleLoadAbortEvent(event, webViewEvent);
    },
    evt: CreateEvent('webview.onLoadAbort'),
    fields: ['url', 'isTopLevel', 'reason']
  },
  'loadcommit': {
    customHandler: function(webViewInternal, event, webViewEvent) {
      webViewInternal.handleLoadCommitEvent(event, webViewEvent);
    },
    evt: CreateEvent('webview.onLoadCommit'),
    fields: ['url', 'isTopLevel']
  },
  'loadprogress': {
    evt: CreateEvent('webview.onLoadProgress'),
    fields: ['url', 'progress']
  },
  'loadredirect': {
    evt: CreateEvent('webview.onLoadRedirect'),
    fields: ['isTopLevel', 'oldUrl', 'newUrl']
  },
  'loadstart': {
    evt: CreateEvent('webview.onLoadStart'),
    fields: ['url', 'isTopLevel']
  },
  'loadstop': {
    evt: CreateEvent('webview.onLoadStop'),
    fields: []
  },
  'newwindow': {
    cancelable: true,
    customHandler: function(webViewInternal, event, webViewEvent) {
      webViewInternal.handleNewWindowEvent(event, webViewEvent);
    },
    evt: CreateEvent('webview.onNewWindow'),
    fields: [
      'initialHeight',
      'initialWidth',
      'targetUrl',
      'windowOpenDisposition',
      'name'
    ]
  },
  'permissionrequest': {
    cancelable: true,
    customHandler: function(webViewInternal, event, webViewEvent) {
      webViewInternal.handlePermissionEvent(event, webViewEvent);
    },
    evt: CreateEvent('webview.onPermissionRequest'),
    fields: [
      'identifier',
      'lastUnlockedBySelf',
      'name',
      'permission',
      'requestMethod',
      'url',
      'userGesture'
    ]
  },
  'responsive': {
    evt: CreateEvent('webview.onResponsive'),
    fields: ['processId']
  },
  'sizechanged': {
    evt: CreateEvent('webview.onSizeChanged'),
    customHandler: function(webViewInternal, event, webViewEvent) {
      webViewInternal.handleSizeChangedEvent(event, webViewEvent);
    },
    fields: ['oldHeight', 'oldWidth', 'newHeight', 'newWidth']
  },
  'unresponsive': {
    evt: CreateEvent('webview.onUnresponsive'),
    fields: ['processId']
  }
};

// Implemented when the experimental API is available.
WebViewInternal.maybeRegisterExperimentalAPIs = function(proto) {}

/**
 * @constructor
 */
function WebViewInternal(webviewNode) {
  privates(webviewNode).internal = this;
  this.webviewNode = webviewNode;
  this.browserPluginNode = this.createBrowserPluginNode();
  var shadowRoot = this.webviewNode.createShadowRoot();
  shadowRoot.appendChild(this.browserPluginNode);

  this.setupWebviewNodeAttributes();
  this.setupFocusPropagation();
  this.setupWebviewNodeProperties();
  this.setupWebviewNodeEvents();
}

/**
 * @private
 */
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
  var args = $Array.concat([this.instanceId], $Array.slice(arguments));
  $Function.apply(WebView.executeScript, null, args);
};

/**
 * @private
 */
WebViewInternal.prototype.insertCSS = function(var_args) {
  this.validateExecuteCodeCall();
  var args = $Array.concat([this.instanceId], $Array.slice(arguments));
  $Function.apply(WebView.insertCSS, null, args);
};

/**
 * @private
 */
WebViewInternal.prototype.setupWebviewNodeProperties = function() {
  var ERROR_MSG_CONTENTWINDOW_NOT_AVAILABLE = '<webview>: ' +
    'contentWindow is not available at this time. It will become available ' +
        'when the page has finished loading.';

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
  this.srcObserver = new MutationObserver(function(mutations) {
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
    attributeFilter: ['src']
  };
  this.srcObserver.observe(this.webviewNode, params);
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
  if (name == 'src') {
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
      this.srcObserver.takeRecords();
      this.ignoreNextSrcAttributeChange = false;
      return;
    }
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

/**
 * @private
 */
WebViewInternal.prototype.getEvents = function() {
  var experimentalEvents = this.maybeGetExperimentalEvents();
  for (var eventName in experimentalEvents) {
    WEB_VIEW_EVENTS[eventName] = experimentalEvents[eventName];
  }
  return WEB_VIEW_EVENTS;
};

WebViewInternal.prototype.handleSizeChangedEvent =
    function(event, webViewEvent) {
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

  if (webViewEvent.newWidth >= minWidth &&
      webViewEvent.newWidth <= maxWidth &&
      webViewEvent.newHeight >= minHeight &&
      webViewEvent.newHeight <= maxHeight) {
    node.style.width = webViewEvent.newWidth + 'px';
    node.style.height = webViewEvent.newHeight + 'px';
  }
  node.dispatchEvent(webViewEvent);
};

/**
 * @private
 */
WebViewInternal.prototype.setupWebviewNodeEvents = function() {
  var self = this;
  this.viewInstanceId = IdGenerator.GetNextId();
  var onInstanceIdAllocated = function(e) {
    var detail = e.detail ? JSON.parse(e.detail) : {};
    self.instanceId = detail.windowId;
    var params = {
      'api': 'webview',
      'instanceId': self.viewInstanceId
    };
    if (self.userAgentOverride) {
      params['userAgentOverride'] = self.userAgentOverride;
    }
    self.browserPluginNode['-internal-attach'](params);

    var events = self.getEvents();
    for (var eventName in events) {
      self.setupEvent(eventName, events[eventName]);
    }
  };
  this.browserPluginNode.addEventListener('-internal-instanceid-allocated',
                                          onInstanceIdAllocated);
  this.setupWebRequestEvents();
  this.setupExperimentalContextMenus_();

  this.on = {};
  var events = self.getEvents();
  for (var eventName in events) {
    this.setupEventProperty(eventName);
  }
};

/**
 * @private
 */
WebViewInternal.prototype.setupEvent = function(eventName, eventInfo) {
  var self = this;
  var webviewNode = this.webviewNode;
  eventInfo.evt.addListener(function(event) {
    var details = {bubbles:true};
    if (eventInfo.cancelable)
      details.cancelable = true;
    var webViewEvent = new Event(eventName, details);
    $Array.forEach(eventInfo.fields, function(field) {
      if (event[field] !== undefined) {
        webViewEvent[field] = event[field];
      }
    });
    if (eventInfo.customHandler) {
      eventInfo.customHandler(self, event, webViewEvent);
      return;
    }
    webviewNode.dispatchEvent(webViewEvent);
  }, {instanceId: self.instanceId});
};

/**
 * Adds an 'on<event>' property on the webview, which can be used to set/unset
 * an event handler.
 * @private
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

/**
 * @private
 */
WebViewInternal.prototype.getPermissionTypes = function() {
  var permissions =
      ['media', 'geolocation', 'pointerLock', 'download', 'loadplugin'];
  return permissions.concat(this.maybeGetExperimentalPermissions());
};

/**
 * @private
 */
WebViewInternal.prototype.handleDialogEvent =
    function(event, webViewEvent) {
  var showWarningMessage = function(dialogType) {
    var VOWELS = ['a', 'e', 'i', 'o', 'u'];
    var WARNING_MSG_DIALOG_BLOCKED = '<webview>: %1 %2 dialog was blocked.';
    var article = (VOWELS.indexOf(dialogType.charAt(0)) >= 0) ? 'An' : 'A';
    var output = WARNING_MSG_DIALOG_BLOCKED.replace('%1', article);
    output = output.replace('%2', dialogType);
    window.console.warn(output);
  };

  var self = this;
  var browserPluginNode = this.browserPluginNode;
  var webviewNode = this.webviewNode;

  var requestId = event.requestId;
  var actionTaken = false;

  var validateCall = function() {
    var ERROR_MSG_DIALOG_ACTION_ALREADY_TAKEN = '<webview>: ' +
        'An action has already been taken for this "dialog" event.';

    if (actionTaken) {
      throw new Error(ERROR_MSG_DIALOG_ACTION_ALREADY_TAKEN);
    }
    actionTaken = true;
  };

  var dialog = {
    ok: function(user_input) {
      validateCall();
      user_input = user_input || '';
      WebView.setPermission(self.instanceId, requestId, 'allow', user_input);
    },
    cancel: function() {
      validateCall();
      WebView.setPermission(self.instanceId, requestId, 'deny');
    }
  };
  webViewEvent.dialog = dialog;

  var defaultPrevented = !webviewNode.dispatchEvent(webViewEvent);
  if (actionTaken) {
    return;
  }

  if (defaultPrevented) {
    // Tell the JavaScript garbage collector to track lifetime of |dialog| and
    // call back when the dialog object has been collected.
    MessagingNatives.BindToGC(dialog, function() {
      // Avoid showing a warning message if the decision has already been made.
      if (actionTaken) {
        return;
      }
      WebView.setPermission(
          self.instanceId, requestId, 'default', '', function(allowed) {
        if (allowed) {
          return;
        }
        showWarningMessage(event.messageType);
      });
    });
  } else {
    actionTaken = true;
    // The default action is equivalent to canceling the dialog.
    WebView.setPermission(
        self.instanceId, requestId, 'default', '', function(allowed) {
      if (allowed) {
        return;
      }
      showWarningMessage(event.messageType);
    });
  }
};

/**
 * @private
 */
WebViewInternal.prototype.handleLoadAbortEvent =
    function(event, webViewEvent) {
  var showWarningMessage = function(reason) {
    var WARNING_MSG_LOAD_ABORTED = '<webview>: ' +
        'The load has aborted with reason "%1".';
    window.console.warn(WARNING_MSG_LOAD_ABORTED.replace('%1', reason));
  };
  if (this.webviewNode.dispatchEvent(webViewEvent)) {
    showWarningMessage(event.reason);
  }
};

/**
 * @private
 */
WebViewInternal.prototype.handleLoadCommitEvent =
    function(event, webViewEvent) {
  this.currentEntryIndex = event.currentEntryIndex;
  this.entryCount = event.entryCount;
  this.processId = event.processId;
  var oldValue = this.webviewNode.getAttribute('src');
  var newValue = event.url;
  if (event.isTopLevel && (oldValue != newValue)) {
    // Touching the src attribute triggers a navigation. To avoid
    // triggering a page reload on every guest-initiated navigation,
    // we use the flag ignoreNextSrcAttributeChange here.
    this.ignoreNextSrcAttributeChange = true;
    this.webviewNode.setAttribute('src', newValue);
  }
  this.webviewNode.dispatchEvent(webViewEvent);
}

/**
 * @private
 */
WebViewInternal.prototype.handleNewWindowEvent =
    function(event, webViewEvent) {
  var ERROR_MSG_NEWWINDOW_ACTION_ALREADY_TAKEN = '<webview>: ' +
      'An action has already been taken for this "newwindow" event.';

  var ERROR_MSG_NEWWINDOW_UNABLE_TO_ATTACH = '<webview>: ' +
      'Unable to attach the new window to the provided webview.';

  var ERROR_MSG_WEBVIEW_EXPECTED = '<webview> element expected.';

  var showWarningMessage = function() {
    var WARNING_MSG_NEWWINDOW_BLOCKED = '<webview>: A new window was blocked.';
    window.console.warn(WARNING_MSG_NEWWINDOW_BLOCKED);
  };

  var self = this;
  var browserPluginNode = this.browserPluginNode;
  var webviewNode = this.webviewNode;

  var requestId = event.requestId;
  var actionTaken = false;

  var validateCall = function () {
    if (actionTaken) {
      throw new Error(ERROR_MSG_NEWWINDOW_ACTION_ALREADY_TAKEN);
    }
    actionTaken = true;
  };

  var windowObj = {
    attach: function(webview) {
      validateCall();
      if (!webview)
        throw new Error(ERROR_MSG_WEBVIEW_EXPECTED);
      // Attach happens asynchronously to give the tagWatcher an opportunity
      // to pick up the new webview before attach operates on it, if it hasn't
      // been attached to the DOM already.
      // Note: Any subsequent errors cannot be exceptions because they happen
      // asynchronously.
      setTimeout(function() {
        var attached =
            browserPluginNode['-internal-attachWindowTo'](webview,
                                                          event.windowId);
        if (!attached) {
          window.console.error(ERROR_MSG_NEWWINDOW_UNABLE_TO_ATTACH);
        }
        // If the object being passed into attach is not a valid <webview>
        // then we will fail and it will be treated as if the new window
        // was rejected. The permission API plumbing is used here to clean
        // up the state created for the new window if attaching fails.
        WebView.setPermission(
            self.instanceId, requestId, attached ? 'allow' : 'deny');
      }, 0);
    },
    discard: function() {
      validateCall();
      WebView.setPermission(self.instanceId, requestId, 'deny');
    }
  };
  webViewEvent.window = windowObj;

  var defaultPrevented = !webviewNode.dispatchEvent(webViewEvent);
  if (actionTaken) {
    return;
  }

  if (defaultPrevented) {
    // Make browser plugin track lifetime of |windowObj|.
    MessagingNatives.BindToGC(windowObj, function() {
      // Avoid showing a warning message if the decision has already been made.
      if (actionTaken) {
        return;
      }
      WebView.setPermission(
          self.instanceId, requestId, 'default', '', function(allowed) {
        if (allowed) {
          return;
        }
        showWarningMessage();
      });
    });
  } else {
    actionTaken = true;
    // The default action is to discard the window.
    WebView.setPermission(
        self.instanceId, requestId, 'default', '', function(allowed) {
      if (allowed) {
        return;
      }
      showWarningMessage();
    });
  }
};

WebViewInternal.prototype.handlePermissionEvent =
    function(event, webViewEvent) {
  var ERROR_MSG_PERMISSION_ALREADY_DECIDED = '<webview>: ' +
      'Permission has already been decided for this "permissionrequest" event.';

  var showWarningMessage = function(permission) {
    var WARNING_MSG_PERMISSION_DENIED = '<webview>: ' +
        'The permission request for "%1" has been denied.';
    window.console.warn(
        WARNING_MSG_PERMISSION_DENIED.replace('%1', permission));
  };

  var requestId = event.requestId;
  var self = this;

  if (this.getPermissionTypes().indexOf(event.permission) < 0) {
    // The permission type is not allowed. Trigger the default response.
    WebView.setPermission(
        self.instanceId, requestId, 'default', '', function(allowed) {
      if (allowed) {
        return;
      }
      showWarningMessage(event.permission);
    });
    return;
  }

  var browserPluginNode = this.browserPluginNode;
  var webviewNode = this.webviewNode;

  var decisionMade = false;

  var validateCall = function() {
    if (decisionMade) {
      throw new Error(ERROR_MSG_PERMISSION_ALREADY_DECIDED);
    }
    decisionMade = true;
  };

  // Construct the event.request object.
  var request = {
    allow: function() {
      validateCall();
      WebView.setPermission(self.instanceId, requestId, 'allow');
    },
    deny: function() {
      validateCall();
      WebView.setPermission(self.instanceId, requestId, 'deny');
    }
  };
  webViewEvent.request = request;

  var defaultPrevented = !webviewNode.dispatchEvent(webViewEvent);
  if (decisionMade) {
    return;
  }

  if (defaultPrevented) {
    // Make browser plugin track lifetime of |request|.
    MessagingNatives.BindToGC(request, function() {
      // Avoid showing a warning message if the decision has already been made.
      if (decisionMade) {
        return;
      }
      WebView.setPermission(
          self.instanceId, requestId, 'default', '', function(allowed) {
        if (allowed) {
          return;
        }
        showWarningMessage(event.permission);
      });
    });
  } else {
    decisionMade = true;
    WebView.setPermission(
        self.instanceId, requestId, 'default', '', function(allowed) {
      if (allowed) {
        return;
      }
      showWarningMessage(event.permission);
    });
  }
};

/**
 * @private
 */
WebViewInternal.prototype.setupWebRequestEvents = function() {
  var self = this;
  var request = {};
  var createWebRequestEvent = function(webRequestEvent) {
    return function() {
      if (!self[webRequestEvent.name]) {
        self[webRequestEvent.name] =
            new WebRequestEvent(
                'webview.' + webRequestEvent.name,
                webRequestEvent.parameters,
                webRequestEvent.extraParameters, webRequestEvent.options,
                self.viewInstanceId);
      }
      return self[webRequestEvent.name];
    };
  };

  for (var i = 0; i < DeclarativeWebRequestSchema.events.length; ++i) {
    var eventSchema = DeclarativeWebRequestSchema.events[i];
    var webRequestEvent = createWebRequestEvent(eventSchema);
    this.maybeAttachWebRequestEventToObject(request,
                                            eventSchema.name,
                                            webRequestEvent);
  }

  // Populate the WebRequest events from the API definition.
  for (var i = 0; i < WebRequestSchema.events.length; ++i) {
    var webRequestEvent = createWebRequestEvent(WebRequestSchema.events[i]);
    Object.defineProperty(
        request,
        WebRequestSchema.events[i].name,
        {
          get: webRequestEvent,
          enumerable: true
        }
    );
    this.maybeAttachWebRequestEventToObject(this.webviewNode,
                                            WebRequestSchema.events[i].name,
                                            webRequestEvent);
  }
  Object.defineProperty(
      this.webviewNode,
      'request',
      {
        value: request,
        enumerable: true
      }
  );
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
    internal.handleBrowserPluginAttributeMutation(name, newValue);
  };

  proto.attachedCallback = function() {
    // Load the plugin immediately.
    var unused = this.nonExistentAttribute;
  };

  WebViewInternal.BrowserPlugin =
      DocumentNatives.RegisterElement('browser-plugin', {extends: 'object',
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

  proto.back = function() {
    this.go(-1);
  };

  proto.forward = function() {
    this.go(1);
  };

  proto.canGoBack = function() {
    return privates(this).internal.canGoBack();
  };

  proto.canGoForward = function() {
    return privates(this).internal.canGoForward();
  };

  proto.clearData = function() {
    var internal = privates(this).internal;
    $Function.apply(internal.clearData, internal, arguments);
  };

  proto.getProcessId = function() {
    return privates(this).internal.getProcessId();
  };

  proto.go = function(relativeIndex) {
    privates(this).internal.go(relativeIndex);
  };

  proto.reload = function() {
    privates(this).internal.reload();
  };

  proto.stop = function() {
    privates(this).internal.stop();
  };

  proto.terminate = function() {
    privates(this).internal.terminate();
  };

  proto.executeScript = function(var_args) {
    var internal = privates(this).internal;
    $Function.apply(internal.executeScript, internal, arguments);
  };

  proto.insertCSS = function(var_args) {
    var internal = privates(this).internal;
    $Function.apply(internal.insertCSS, internal, arguments);
  };

  proto.getUserAgent = function() {
    return privates(this).internal.getUserAgent();
  };

  proto.isUserAgentOverridden = function() {
    return privates(this).internal.isUserAgentOverridden();
  };

  proto.setUserAgentOverride = function(userAgentOverride) {
    privates(this).internal.setUserAgentOverride(userAgentOverride);
  };
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
WebViewInternal.prototype.maybeAttachWebRequestEventToObject = function() {};

/**
 * Implemented when the experimental API is available.
 * @private
 */
WebViewInternal.prototype.maybeGetExperimentalPermissions = function() {
  return [];
};

/**
 * Implemented when the experimental API is available.
 * @private
 */
WebViewInternal.prototype.setupExperimentalContextMenus_ = function() {};

exports.WebView = WebView;
exports.WebViewInternal = WebViewInternal;
exports.CreateEvent = CreateEvent;
