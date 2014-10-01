// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This module implements experimental API for <webview>.
// See web_view.js for details.
//
// <webview> Experimental API is only available on canary and dev channels of
// Chrome.

var ContextMenusSchema =
    requireNative('schema_registry').GetSchema('contextMenus');
var CreateEvent = require('webView').CreateEvent;
var EventBindings = require('event_bindings');
var MessagingNatives = requireNative('messaging_natives');
var WebView = require('webView').WebView;
var WebViewInternal = require('webView').WebViewInternal;
var WebViewSchema = requireNative('schema_registry').GetSchema('webview');
var idGeneratorNatives = requireNative('id_generator');
var utils = require('utils');

// WEB_VIEW_EXPERIMENTAL_EVENTS is a map of experimental <webview> DOM event
//     names to their associated extension event descriptor objects.
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
var WEB_VIEW_EXPERIMENTAL_EVENTS = {
  'findupdate': {
    evt: CreateEvent('webview.onFindReply'),
    fields: [
      'searchText',
      'numberOfMatches',
      'activeMatchOrdinal',
      'selectionRect',
      'canceled',
      'finalUpdate'
    ]
  },
  'zoomchange': {
    evt: CreateEvent('webview.onZoomChange'),
    fields: ['oldZoomFactor', 'newZoomFactor']
  }
};

function GetUniqueSubEventName(eventName) {
  return eventName + "/" + idGeneratorNatives.GetNextId();
}

// This is the only "webview.onClicked" named event for this renderer.
//
// Since we need an event per <webview>, we define events with suffix
// (subEventName) in each of the <webview>. Behind the scenes, this event is
// registered as a ContextMenusEvent, with filter set to the webview's
// |viewInstanceId|. Any time a ContextMenusEvent is dispatched, we re-dispatch
// it to the subEvent's listeners. This way
// <webview>.contextMenus.onClicked behave as a regular chrome Event type.
var ContextMenusEvent = CreateEvent('webview.onClicked');

/**
 * This event is exposed as <webview>.contextMenus.onClicked.
 *
 * @constructor
 */
function ContextMenusOnClickedEvent(opt_eventName,
                                    opt_argSchemas,
                                    opt_eventOptions,
                                    opt_webViewInstanceId) {
  var subEventName = GetUniqueSubEventName(opt_eventName);
  EventBindings.Event.call(this, subEventName, opt_argSchemas, opt_eventOptions,
      opt_webViewInstanceId);

  var self = this;
  // TODO(lazyboy): When do we dispose this listener?
  ContextMenusEvent.addListener(function() {
    // Re-dispatch to subEvent's listeners.
    $Function.apply(self.dispatch, self, $Array.slice(arguments));
  }, {instanceId: opt_webViewInstanceId || 0});
}

ContextMenusOnClickedEvent.prototype = {
  __proto__: EventBindings.Event.prototype
};

/**
 * An instance of this class is exposed as <webview>.contextMenus.
 * @constructor
 */
function WebViewContextMenusImpl(viewInstanceId) {
  this.viewInstanceId_ = viewInstanceId;
};

WebViewContextMenusImpl.prototype.create = function() {
  var args = $Array.concat([this.viewInstanceId_], $Array.slice(arguments));
  return $Function.apply(WebView.contextMenusCreate, null, args);
};

WebViewContextMenusImpl.prototype.remove = function() {
  var args = $Array.concat([this.viewInstanceId_], $Array.slice(arguments));
  return $Function.apply(WebView.contextMenusRemove, null, args);
};

WebViewContextMenusImpl.prototype.removeAll = function() {
  var args = $Array.concat([this.viewInstanceId_], $Array.slice(arguments));
  return $Function.apply(WebView.contextMenusRemoveAll, null, args);
};

WebViewContextMenusImpl.prototype.update = function() {
  var args = $Array.concat([this.viewInstanceId_], $Array.slice(arguments));
  return $Function.apply(WebView.contextMenusUpdate, null, args);
};

var WebViewContextMenus = utils.expose(
    'WebViewContextMenus', WebViewContextMenusImpl,
    ['create', 'remove', 'removeAll', 'update']);

/**
 * @private
 */
WebViewInternal.prototype.maybeAttachWebRequestEventToObject =
    function(obj, eventName, webRequestEvent) {
  Object.defineProperty(
      obj,
      eventName,
      {
        get: webRequestEvent,
        enumerable: true
      }
  );
};

/**
 * @private
 */
WebViewInternal.prototype.setZoom = function(zoomFactor) {
  if (!this.instanceId) {
    return;
  }
  WebView.setZoom(this.instanceId, zoomFactor);
};

WebViewInternal.prototype.maybeGetExperimentalEvents = function() {
  return WEB_VIEW_EXPERIMENTAL_EVENTS;
};

/** @private */
WebViewInternal.prototype.maybeGetExperimentalPermissions = function() {
  return [];
};

/** @private */
WebViewInternal.prototype.maybeSetCurrentZoomFactor =
    function(zoomFactor) {
  this.currentZoomFactor = zoomFactor;
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

/** @private */
WebViewInternal.prototype.captureVisibleRegion = function(spec, callback) {
  WebView.captureVisibleRegion(this.instanceId, spec, callback);
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

WebViewInternal.maybeRegisterExperimentalAPIs = function(proto) {
  proto.setZoom = function(zoomFactor, callback) {
    privates(this).internal.setZoom(zoomFactor, callback);
  };

  proto.getZoom = function(callback) {
    return privates(this).internal.getZoom(callback);
  };

  proto.captureVisibleRegion = function(spec, callback) {
    privates(this).internal.captureVisibleRegion(spec, callback);
  };

  proto.find = function(search_text, options, callback) {
    privates(this).internal.find(search_text, options, callback);
  };

  proto.stopFinding = function(action) {
    privates(this).internal.stopFinding(action);
  };
};

/** @private */
WebViewInternal.prototype.setupExperimentalContextMenus_ = function() {
  var self = this;
  var createContextMenus = function() {
    return function() {
      if (self.contextMenus_) {
        return self.contextMenus_;
      }

      self.contextMenus_ = new WebViewContextMenus(self.viewInstanceId);

      // Define 'onClicked' event property on |self.contextMenus_|.
      var getOnClickedEvent = function() {
        return function() {
          if (!self.contextMenusOnClickedEvent_) {
            var eventName = 'webview.onClicked';
            // TODO(lazyboy): Find event by name instead of events[0].
            var eventSchema = WebViewSchema.events[0];
            var eventOptions = {supportsListeners: true};
            var onClickedEvent = new ContextMenusOnClickedEvent(
                eventName, eventSchema, eventOptions, self.viewInstanceId);
            self.contextMenusOnClickedEvent_ = onClickedEvent;
            return onClickedEvent;
          }
          return self.contextMenusOnClickedEvent_;
        }
      };
      Object.defineProperty(
          self.contextMenus_,
          'onClicked',
          {get: getOnClickedEvent(), enumerable: true});

      return self.contextMenus_;
    };
  };

  // Expose <webview>.contextMenus object.
  Object.defineProperty(
      this.webviewNode,
      'contextMenus',
      {
        get: createContextMenus(),
        enumerable: true
      });
};
