// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Custom binding for the webViewRequest API.

var binding = require('binding').Binding.create('webViewRequest');

var declarativeWebRequestSchema =
    requireNative('schema_registry').GetSchema('declarativeWebRequest');
var utils = require('utils');
var validate = require('schemaUtils').validate;

binding.registerCustomHook(function(api) {
  var webViewRequest = api.compiledApi;

  // Returns the schema definition of type |typeId| defined in
  // |declarativeWebRequestScheme.types|.
  function getSchema(typeId) {
    return utils.lookup(declarativeWebRequestSchema.types,
                        'id',
                        'declarativeWebRequest.' + typeId);
  }

  // Helper function for the constructor of concrete datatypes of the
  // declarative webRequest API.
  // Makes sure that |this| contains the union of parameters and
  // {'instanceType': 'declarativeWebRequest.' + typeId} and validates the
  // generated union dictionary against the schema for |typeId|.
  function setupInstance(instance, parameters, typeId) {
    for (var key in parameters) {
      if ($Object.hasOwnProperty(parameters, key)) {
        instance[key] = parameters[key];
      }
    }

    instance.instanceType = 'declarativeWebRequest.' + typeId;
    var schema = getSchema(typeId);
    validate([instance], [schema]);
  }

  // Setup all data types for the declarative webRequest API from the schema.
  for (var i = 0; i < declarativeWebRequestSchema.types.length; ++i) {
    var typeSchema = declarativeWebRequestSchema.types[i];
    var typeId = typeSchema.id.replace('declarativeWebRequest.', '');
    var action = function(typeId) {
      return function(parameters) {
        setupInstance(this, parameters, typeId);
      };
    }(typeId);
    webViewRequest[typeId] = action;
  }
});

exports.binding = binding.generate();
