// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the webRequest API.

var binding = require('binding').Binding.create('webRequest');
var sendRequest = require('sendRequest').sendRequest;
var WebRequestEvent = require('webRequestInternal').WebRequestEvent;

binding.registerCustomHook(function(api) {
  var apiFunctions = api.apiFunctions;

  apiFunctions.setHandleRequest('handlerBehaviorChanged', function() {
    var args = $Array.slice(arguments);
    sendRequest(this.name, args, this.definition.parameters,
                {forIOThread: true});
  });
});

binding.registerCustomEvent(WebRequestEvent);

exports.binding = binding.generate();
