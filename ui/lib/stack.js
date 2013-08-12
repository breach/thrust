/*
 * Breach: stack.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-12 spolu   Creation
 */

var common = require('./common.js');
var factory = common.factory;
var api = require('./api.js');

//
// ### stack
//
// ```
// @spec { socket }
// ```
//
var stack = function(spec, my) {
  var _super = {};
  my = my || {};
  spec = spec || {};

  my.socket = spec.socket || null;

  /* { name, date, title, last_url, navigation } */
  my.history = [];

  //
  // ### _public_
  //
  var page_navigate;   /* page_navigate(frame, url); */
  var page_title;      /* page_title(frame, title); */
  
  //
  // ### _private_
  //
  var init;    /* init(); */

  //
  // #### _that_
  //
  var that = {};

  my.socket.on('history', function(query, cb_) {
    var count = query.count || 20;
  });

  my.socket.on('search', function(query, cb_) {
  });

  my.socket.on('new_page', function() {
  });

  init = function() {
    my.socket.emit('ready');
  };

  init();
  
  return that;
};

