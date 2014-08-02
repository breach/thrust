// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the webRequestInternal API.

var binding = require('binding').Binding.create('webRequestInternal');
var eventBindings = require('event_bindings');
var sendRequest = require('sendRequest').sendRequest;
var validate = require('schemaUtils').validate;
var utils = require('utils');
var idGeneratorNatives = requireNative('id_generator');

var webRequestInternal;

function GetUniqueSubEventName(eventName) {
  return eventName + "/" + idGeneratorNatives.GetNextId();
}

// WebRequestEventImpl object. This is used for special webRequest events
// with extra parameters. Each invocation of addListener creates a new named
// sub-event. That sub-event is associated with the extra parameters in the
// browser process, so that only it is dispatched when the main event occurs
// matching the extra parameters.
//
// Example:
//   chrome.webRequest.onBeforeRequest.addListener(
//       callback, {urls: 'http://*.google.com/*'});
//   ^ callback will only be called for onBeforeRequests matching the filter.
function WebRequestEventImpl(eventName, opt_argSchemas, opt_extraArgSchemas,
                             opt_eventOptions, opt_webViewInstanceId) {
  if (typeof eventName != 'string')
    throw new Error('chrome.WebRequestEvent requires an event name.');

  this.eventName = eventName;
  this.argSchemas = opt_argSchemas;
  this.extraArgSchemas = opt_extraArgSchemas;
  this.webViewInstanceId = opt_webViewInstanceId || 0;
  this.subEvents = [];
  this.eventOptions = eventBindings.parseEventOptions(opt_eventOptions);
  if (this.eventOptions.supportsRules) {
    this.eventForRules =
        new eventBindings.Event(eventName, opt_argSchemas, opt_eventOptions,
                                opt_webViewInstanceId);
  }
}

// Test if the given callback is registered for this event.
WebRequestEventImpl.prototype.hasListener = function(cb) {
  if (!this.eventOptions.supportsListeners)
    throw new Error('This event does not support listeners.');
  return this.findListener_(cb) > -1;
};

// Test if any callbacks are registered fur thus event.
WebRequestEventImpl.prototype.hasListeners = function() {
  if (!this.eventOptions.supportsListeners)
    throw new Error('This event does not support listeners.');
  return this.subEvents.length > 0;
};

// Registers a callback to be called when this event is dispatched. If
// opt_filter is specified, then the callback is only called for events that
// match the given filters. If opt_extraInfo is specified, the given optional
// info is sent to the callback.
WebRequestEventImpl.prototype.addListener =
    function(cb, opt_filter, opt_extraInfo) {
  if (!this.eventOptions.supportsListeners)
    throw new Error('This event does not support listeners.');
  // NOTE(benjhayden) New APIs should not use this subEventName trick! It does
  // not play well with event pages. See downloads.onDeterminingFilename and
  // ExtensionDownloadsEventRouter for an alternative approach.
  var subEventName = GetUniqueSubEventName(this.eventName);
  // Note: this could fail to validate, in which case we would not add the
  // subEvent listener.
  validate($Array.slice(arguments, 1), this.extraArgSchemas);
  webRequestInternal.addEventListener(
      cb, opt_filter, opt_extraInfo, this.eventName, subEventName,
      this.webViewInstanceId);

  var subEvent = new eventBindings.Event(subEventName, this.argSchemas);
  var subEventCallback = cb;
  if (opt_extraInfo && opt_extraInfo.indexOf('blocking') >= 0) {
    var eventName = this.eventName;
    subEventCallback = function() {
      var requestId = arguments[0].requestId;
      try {
        var result = $Function.apply(cb, null, arguments);
        webRequestInternal.eventHandled(
            eventName, subEventName, requestId, result);
      } catch (e) {
        webRequestInternal.eventHandled(
            eventName, subEventName, requestId);
        throw e;
      }
    };
  } else if (opt_extraInfo && opt_extraInfo.indexOf('asyncBlocking') >= 0) {
    var eventName = this.eventName;
    subEventCallback = function() {
      var details = arguments[0];
      var requestId = details.requestId;
      var handledCallback = function(response) {
        webRequestInternal.eventHandled(
            eventName, subEventName, requestId, response);
      };
      $Function.apply(cb, null, [details, handledCallback]);
    };
  }
  $Array.push(this.subEvents,
      {subEvent: subEvent, callback: cb, subEventCallback: subEventCallback});
  subEvent.addListener(subEventCallback);
};

// Unregisters a callback.
WebRequestEventImpl.prototype.removeListener = function(cb) {
  if (!this.eventOptions.supportsListeners)
    throw new Error('This event does not support listeners.');
  var idx;
  while ((idx = this.findListener_(cb)) >= 0) {
    var e = this.subEvents[idx];
    e.subEvent.removeListener(e.subEventCallback);
    if (e.subEvent.hasListeners()) {
      console.error(
          'Internal error: webRequest subEvent has orphaned listeners.');
    }
    $Array.splice(this.subEvents, idx, 1);
  }
};

WebRequestEventImpl.prototype.findListener_ = function(cb) {
  for (var i in this.subEvents) {
    var e = this.subEvents[i];
    if (e.callback === cb) {
      if (e.subEvent.hasListener(e.subEventCallback))
        return i;
      console.error('Internal error: webRequest subEvent has no callback.');
    }
  }

  return -1;
};

WebRequestEventImpl.prototype.addRules = function(rules, opt_cb) {
  if (!this.eventOptions.supportsRules)
    throw new Error('This event does not support rules.');
  this.eventForRules.addRules(rules, opt_cb);
};

WebRequestEventImpl.prototype.removeRules =
    function(ruleIdentifiers, opt_cb) {
  if (!this.eventOptions.supportsRules)
    throw new Error('This event does not support rules.');
  this.eventForRules.removeRules(ruleIdentifiers, opt_cb);
};

WebRequestEventImpl.prototype.getRules = function(ruleIdentifiers, cb) {
  if (!this.eventOptions.supportsRules)
    throw new Error('This event does not support rules.');
  this.eventForRules.getRules(ruleIdentifiers, cb);
};

binding.registerCustomHook(function(api) {
  var apiFunctions = api.apiFunctions;

  apiFunctions.setHandleRequest('addEventListener', function() {
    var args = $Array.slice(arguments);
    sendRequest(this.name, args, this.definition.parameters,
                {forIOThread: true});
  });

  apiFunctions.setHandleRequest('eventHandled', function() {
    var args = $Array.slice(arguments);
    sendRequest(this.name, args, this.definition.parameters,
                {forIOThread: true});
  });
});

var WebRequestEvent = utils.expose('WebRequestEvent', WebRequestEventImpl, [
  'hasListener',
  'hasListeners',
  'addListener',
  'removeListener',
  'addRules',
  'removeRules',
  'getRules'
]);

webRequestInternal = binding.generate();
exports.binding = webRequestInternal;
exports.WebRequestEvent = WebRequestEvent;
