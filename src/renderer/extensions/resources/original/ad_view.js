// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Shim that simulates a <adview> tag via Mutation Observers.
//
// The actual tag is implemented via the browser plugin. The internals of this
// are hidden via Shadow DOM.

// TODO(rpaquay): This file is currently very similar to "web_view.js". Do we
//                want to refactor to extract common pieces?

var eventBindings = require('event_bindings');
var process = requireNative('process');
var addTagWatcher = require('tagWatcher').addTagWatcher;

/**
 * List of attribute names to "blindly" sync between <adview> tag and internal
 * browser plugin.
 */
var AD_VIEW_ATTRIBUTES = [
  'name',
];

/**
 * List of custom attributes (and their behavior).
 *
 * name: attribute name.
 * onMutation(adview, mutation): callback invoked when attribute is mutated.
 * isProperty: True if the attribute should be exposed as a property.
 */
var AD_VIEW_CUSTOM_ATTRIBUTES = [
  {
    name: 'ad-network',
    onMutation: function(adview, mutation) {
      adview.handleAdNetworkMutation(mutation);
    },
    isProperty: function() {
      return true;
    }
  }
];

/**
 * List of api methods. These are forwarded to the browser plugin.
 */
var AD_VIEW_API_METHODS = [
 // Empty for now.
];

var createEvent = function(name) {
  var eventOpts = {supportsListeners: true, supportsFilters: true};
  return new eventBindings.Event(name, undefined, eventOpts);
};

var AdviewLoadAbortEvent = createEvent('adview.onLoadAbort');
var AdviewLoadCommitEvent = createEvent('adview.onLoadCommit');

var AD_VIEW_EXT_EVENTS = {
  'loadabort': {
    evt: AdviewLoadAbortEvent,
    fields: ['url', 'isTopLevel', 'reason']
  },
  'loadcommit': {
    evt: AdviewLoadCommitEvent,
    fields: ['url', 'isTopLevel']
  }
};

/**
 * List of supported ad-networks.
 *
 * name: identifier of the ad-network, corresponding to a valid value
 *       of the "ad-network" attribute of an <adview> element.
 * url: url to navigate to when initially displaying the <adview>.
 * origin: origin of urls the <adview> is allowed navigate to.
 */
var AD_VIEW_AD_NETWORKS_WHITELIST = [
  {
    name: 'admob',
    url: 'https://admob-sdk.doubleclick.net/chromeapps',
    origin: 'https://double.net'
  },
];

/**
 * Return the whitelisted ad-network entry named |name|.
 */
function getAdNetworkInfo(name) {
  var result = null;
  $Array.forEach(AD_VIEW_AD_NETWORKS_WHITELIST, function(item) {
    if (item.name === name)
      result = item;
  });
  return result;
}

/**
 * @constructor
 */
function AdView(adviewNode) {
  this.adviewNode_ = adviewNode;
  this.browserPluginNode_ = this.createBrowserPluginNode_();
  var shadowRoot = this.adviewNode_.createShadowRoot();
  shadowRoot.appendChild(this.browserPluginNode_);

  this.setupCustomAttributes_();
  this.setupAdviewNodeObservers_();
  this.setupAdviewNodeMethods_();
  this.setupAdviewNodeProperties_();
  this.setupAdviewNodeEvents_();
  this.setupBrowserPluginNodeObservers_();
}

/**
 * @private
 */
AdView.prototype.createBrowserPluginNode_ = function() {
  var browserPluginNode = document.createElement('object');
  browserPluginNode.type = 'application/browser-plugin';
  // The <object> node fills in the <adview> container.
  browserPluginNode.style.width = '100%';
  browserPluginNode.style.height = '100%';
  $Array.forEach(AD_VIEW_ATTRIBUTES, function(attributeName) {
    // Only copy attributes that have been assigned values, rather than copying
    // a series of undefined attributes to BrowserPlugin.
    if (this.adviewNode_.hasAttribute(attributeName)) {
      browserPluginNode.setAttribute(
        attributeName, this.adviewNode_.getAttribute(attributeName));
    }
  }, this);

  return browserPluginNode;
}

/**
 * @private
 */
AdView.prototype.setupCustomAttributes_ = function() {
  $Array.forEach(AD_VIEW_CUSTOM_ATTRIBUTES, function(attributeInfo) {
    if (attributeInfo.onMutation) {
      attributeInfo.onMutation(this);
    }
  }, this);
}

/**
 * @private
 */
AdView.prototype.setupAdviewNodeMethods_ = function() {
  // this.browserPluginNode_[apiMethod] are not necessarily defined immediately
  // after the shadow object is appended to the shadow root.
  var self = this;
  $Array.forEach(AD_VIEW_API_METHODS, function(apiMethod) {
    self.adviewNode_[apiMethod] = function(var_args) {
      return $Function.apply(self.browserPluginNode_[apiMethod],
        self.browserPluginNode_, arguments);
    };
  }, this);
}

/**
 * @private
 */
AdView.prototype.setupAdviewNodeObservers_ = function() {
  // Map attribute modifications on the <adview> tag to property changes in
  // the underlying <object> node.
  var handleMutation = $Function.bind(function(mutation) {
    this.handleAdviewAttributeMutation_(mutation);
  }, this);
  var observer = new MutationObserver(function(mutations) {
    $Array.forEach(mutations, handleMutation);
  });
  observer.observe(
      this.adviewNode_,
      {attributes: true, attributeFilter: AD_VIEW_ATTRIBUTES});

  this.setupAdviewNodeCustomObservers_();
}

/**
 * @private
 */
AdView.prototype.setupAdviewNodeCustomObservers_ = function() {
  var handleMutation = $Function.bind(function(mutation) {
    this.handleAdviewCustomAttributeMutation_(mutation);
  }, this);
  var observer = new MutationObserver(function(mutations) {
    $Array.forEach(mutations, handleMutation);
  });
  var customAttributeNames =
    AD_VIEW_CUSTOM_ATTRIBUTES.map(function(item) { return item.name; });
  observer.observe(
      this.adviewNode_,
      {attributes: true, attributeFilter: customAttributeNames});
}

/**
 * @private
 */
AdView.prototype.setupBrowserPluginNodeObservers_ = function() {
  var handleMutation = $Function.bind(function(mutation) {
    this.handleBrowserPluginAttributeMutation_(mutation);
  }, this);
  var objectObserver = new MutationObserver(function(mutations) {
    $Array.forEach(mutations, handleMutation);
  });
  objectObserver.observe(
      this.browserPluginNode_,
      {attributes: true, attributeFilter: AD_VIEW_ATTRIBUTES});
}

/**
 * @private
 */
AdView.prototype.setupAdviewNodeProperties_ = function() {
  var browserPluginNode = this.browserPluginNode_;
  // Expose getters and setters for the attributes.
  $Array.forEach(AD_VIEW_ATTRIBUTES, function(attributeName) {
    Object.defineProperty(this.adviewNode_, attributeName, {
      get: function() {
        return browserPluginNode[attributeName];
      },
      set: function(value) {
        browserPluginNode[attributeName] = value;
      },
      enumerable: true
    });
  }, this);

  // Expose getters and setters for the custom attributes.
  var adviewNode = this.adviewNode_;
  $Array.forEach(AD_VIEW_CUSTOM_ATTRIBUTES, function(attributeInfo) {
    if (attributeInfo.isProperty()) {
      var attributeName = attributeInfo.name;
      Object.defineProperty(this.adviewNode_, attributeName, {
        get: function() {
          return adviewNode.getAttribute(attributeName);
        },
        set: function(value) {
          adviewNode.setAttribute(attributeName, value);
        },
        enumerable: true
      });
    }
  }, this);

  this.setupAdviewContentWindowProperty_();
}

/**
 * @private
 */
AdView.prototype.setupAdviewContentWindowProperty_ = function() {
  var browserPluginNode = this.browserPluginNode_;
  // We cannot use {writable: true} property descriptor because we want dynamic
  // getter value.
  Object.defineProperty(this.adviewNode_, 'contentWindow', {
    get: function() {
      // TODO(fsamuel): This is a workaround to enable
      // contentWindow.postMessage until http://crbug.com/152006 is fixed.
      if (browserPluginNode.contentWindow)
        return browserPluginNode.contentWindow.self;
      console.error('contentWindow is not available at this time. ' +
          'It will become available when the page has finished loading.');
    },
    // No setter.
    enumerable: true
  });
}

/**
 * @private
 */
AdView.prototype.handleAdviewAttributeMutation_ = function(mutation) {
  // This observer monitors mutations to attributes of the <adview> and
  // updates the BrowserPlugin properties accordingly. In turn, updating
  // a BrowserPlugin property will update the corresponding BrowserPlugin
  // attribute, if necessary. See BrowserPlugin::UpdateDOMAttribute for more
  // details.
  this.browserPluginNode_[mutation.attributeName] =
      this.adviewNode_.getAttribute(mutation.attributeName);
};

/**
 * @private
 */
AdView.prototype.handleAdviewCustomAttributeMutation_ = function(mutation) {
  $Array.forEach(AD_VIEW_CUSTOM_ATTRIBUTES, function(item) {
    if (mutation.attributeName.toUpperCase() == item.name.toUpperCase()) {
      if (item.onMutation) {
        $Function.bind(item.onMutation, item)(this, mutation);
      }
    }
  }, this);
};

/**
 * @private
 */
AdView.prototype.handleBrowserPluginAttributeMutation_ = function(mutation) {
  // This observer monitors mutations to attributes of the BrowserPlugin and
  // updates the <adview> attributes accordingly.
  if (!this.browserPluginNode_.hasAttribute(mutation.attributeName)) {
    // If an attribute is removed from the BrowserPlugin, then remove it
    // from the <adview> as well.
    this.adviewNode_.removeAttribute(mutation.attributeName);
  } else {
    // Update the <adview> attribute to match the BrowserPlugin attribute.
    // Note: Calling setAttribute on <adview> will trigger its mutation
    // observer which will then propagate that attribute to BrowserPlugin. In
    // cases where we permit assigning a BrowserPlugin attribute the same value
    // again (such as navigation when crashed), this could end up in an infinite
    // loop. Thus, we avoid this loop by only updating the <adview> attribute
    // if the BrowserPlugin attributes differs from it.
    var oldValue = this.adviewNode_.getAttribute(mutation.attributeName);
    var newValue = this.browserPluginNode_.getAttribute(mutation.attributeName);
    if (newValue != oldValue) {
      this.adviewNode_.setAttribute(mutation.attributeName, newValue);
    }
  }
};

/**
 * @public
 */
AdView.prototype.handleAdNetworkMutation = function(mutation) {
  if (this.adviewNode_.hasAttribute('ad-network')) {
    var value = this.adviewNode_.getAttribute('ad-network');
    var item = getAdNetworkInfo(value);
    if (!item) {
      // Ignore the new attribute value and set it to empty string.
      // Avoid infinite loop by checking for empty string as new value.
      if (value != '') {
        console.error('The ad-network "' + value + '" is not recognized.');
        this.adviewNode_.setAttribute('ad-network', '');
      }
    }
  }
}

/**
 * @private
 */
AdView.prototype.setupAdviewNodeEvents_ = function() {
  var self = this;
  var onInstanceIdAllocated = function(e) {
    var detail = e.detail ? JSON.parse(e.detail) : {};
    self.instanceId_ = detail.windowId;
    var params = {
      'api': 'adview'
    };
    self.browserPluginNode_['-internal-attach'](params);

    for (var eventName in AD_VIEW_EXT_EVENTS) {
      self.setupExtEvent_(eventName, AD_VIEW_EXT_EVENTS[eventName]);
    }
  };
  this.browserPluginNode_.addEventListener('-internal-instanceid-allocated',
                                           onInstanceIdAllocated);
}

/**
 * @private
 */
AdView.prototype.setupExtEvent_ = function(eventName, eventInfo) {
  var self = this;
  var adviewNode = this.adviewNode_;
  eventInfo.evt.addListener(function(event) {
    var adviewEvent = new Event(eventName, {bubbles: true});
    $Array.forEach(eventInfo.fields, function(field) {
      adviewEvent[field] = event[field];
    });
    if (eventInfo.customHandler) {
      eventInfo.customHandler(self, event);
    }
    adviewNode.dispatchEvent(adviewEvent);
  }, {instanceId: self.instanceId_});
};

/**
 * @public
 */
AdView.prototype.dispatchEvent = function(eventname, detail) {
  // Create event object.
  var evt = new Event(eventname, { bubbles: true });
  for(var item in detail) {
      evt[item] = detail[item];
  }

  // Dispatch event.
  this.adviewNode_.dispatchEvent(evt);
}

addTagWatcher('ADVIEW', function(addedNode) { new AdView(addedNode); });
