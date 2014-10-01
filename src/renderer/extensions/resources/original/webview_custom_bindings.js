// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for <webview> contextMenus API.
// Note that this file mimics custom bindings for chrome.contextMenus API
// which resides in context_menus_custom_bindings.js. The functions in this file
// have an extra instanceId parameter in the beginning, which corresponds to the
// id of the <webview>.
//
// TODO(lazyboy): Share common code /w context_menus_custom_bindings.js.

var EventBindings = require('event_bindings');
var binding = require('binding').Binding.create('webview');
var contextMenuNatives = requireNative('context_menus');
var sendRequest = require('sendRequest').sendRequest;

binding.registerCustomHook(function(bindingsAPI) {
  var apiFunctions = bindingsAPI.apiFunctions;

  var webviewContextMenus = {};
  webviewContextMenus.generatedIdHandlers = {};
  webviewContextMenus.stringIdHandlers = {};

  // Per item event handler.
  var ename = 'webview.contextMenus';
  webviewContextMenus.event = new EventBindings.Event(ename);

  webviewContextMenus.getIdFromCreateProperties = function(prop) {
    if (typeof(prop.id) !== 'undefined')
      return prop.id;
    return prop.generatedId;
  };

  webviewContextMenus.handlersForId = function(instanceId, id) {
    if (typeof(id) === 'number') {
      if (!webviewContextMenus.generatedIdHandlers[instanceId]) {
        webviewContextMenus.generatedIdHandlers[instanceId] = {};
      }
      return webviewContextMenus.generatedIdHandlers[instanceId];
    }

    if (!webviewContextMenus.stringIdHandlers[instanceId]) {
      webviewContextMenus.stringIdHandlers[instanceId] = {};
    }
    return webviewContextMenus.stringIdHandlers[instanceId];
  };

  webviewContextMenus.ensureListenerSetup = function() {
    if (webviewContextMenus.listening) {
      return;
    }
    webviewContextMenus.listening = true;
    webviewContextMenus.event.addListener(function() {
      // An extension context menu item has been clicked on - fire the onclick
      // if there is one.
      var id = arguments[0].menuItemId;
      var instanceId = arguments[0].webviewInstanceId;
      delete arguments[0].webviewInstanceId;
      var onclick = webviewContextMenus.handlersForId(instanceId, id)[id];
      if (onclick) {
        $Function.apply(onclick, null, arguments);
      }
    });
  };

  apiFunctions.setHandleRequest('contextMenusCreate', function() {
    var args = arguments;
    var id = contextMenuNatives.GetNextContextMenuId();
    args[1].generatedId = id;

    var optArgs = {
      customCallback: this.customCallback,
    };

    sendRequest(this.name, args, this.definition.parameters, optArgs);
    return webviewContextMenus.getIdFromCreateProperties(args[1]);
  });

  apiFunctions.setCustomCallback('contextMenusCreate',
                                 function(name, request, response) {
    if (chrome.runtime.lastError) {
      return;
    }

    var instanceId = request.args[0];
    var id = webviewContextMenus.getIdFromCreateProperties(request.args[1]);
    var onclick = request.args.length ? request.args[1].onclick : null;
    if (onclick) {
      webviewContextMenus.ensureListenerSetup();
      webviewContextMenus.handlersForId(instanceId, id)[id] = onclick;
    }
  });

  apiFunctions.setCustomCallback('contextMenusUpdate',
                                 function(name, request, response) {
    if (chrome.runtime.lastError) {
      return;
    }
    var instanceId = request.args[0];
    var id = request.args[1];
    if (request.args[2].onclick) {
      webviewContextMenus.handlersForId(instanceId, id)[id] =
          request.args[2].onclick;
    }
  });

  apiFunctions.setCustomCallback('contextMenusRemove',
                                  function(name, request, response) {
    if (chrome.runtime.lastError) {
      return;
    }
    var instanceId = request.args[0];
    var id = request.args[1];
    delete webviewContextMenus.handlersForId(instanceId, id)[id];
  });

  apiFunctions.setCustomCallback('contextMenusRemoveAll',
                                 function(name, request, response) {
    if (chrome.runtime.lastError) {
      return;
    }
    var instanceId = request.args[0];
    webviewContextMenus.stringIdHandlers[instanceId] = {};
    webviewContextMenus.generatedIdHandlers[instanceId] = {};
  });

});

exports.WebView = binding.generate();
