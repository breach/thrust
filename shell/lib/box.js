/*
 * ExoBrowser: box.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-16 spolu   Creation
 * 2013-09-23 spolu   Simplification for Shell
 */

var common = require('./common.js');
var factory = common.factory;
var api = require('exo_browser');

//
// ### box
//
// ```
// @spec { session }
// ```
//
var box = function(spec, my) {
  var _super = {};
  my = my || {};
  spec = spec || {};

  my.state = {
    value: '',
    can_go_back: false,
    can_go_forward: false,
  };

  //
  // ### _public_
  //
  var init;           /* init(cb_); */
  var handshake;      /* handshake(); */

  //
  // ### _private_
  //
  var push;                    /* push(); */

  var socket_box_input;        /* socket_box_input(input); */
  var socket_box_input_submit; /* socket_box_input_submit(input); */
  var socket_box_input_out;    /* socket_box_input_out(); */

  var socket_box_back;         /* socket_box_back(); */
  var socket_box_forward;      /* socket_box_forward(); */

  //
  // ### _protected_
  //
  var dimension;  /* dimension(); */

  //
  // #### _that_
  //
  var that = require('./control.js').control({
    session: spec.session,
    type: 'box',
    control_type: api.TOP_CONTROL
  }, my);

  /****************************************************************************/
  /*                            CONTROL INTERFACE                             */
  /****************************************************************************/
  // ### dimension
  //  
  // Returns the desired canonical dimension
  dimension = function() {
    return 35;
  };

  // ### handshake
  //
  // Receives the socket and sets up events
  // ```
  // @socket {socket.io socket}
  // ```
  handshake = function(socket) {
    _super.handshake(socket);

    my.socket.on('box_input', socket_box_input);
    my.socket.on('box_input_submit', socket_box_input_submit);
    my.socket.on('box_input_out', socket_box_input_out);

    my.socket.on('box_back', socket_box_back);
    my.socket.on('box_forward', socket_box_forward);

    push();
  };

  // ### init
  // 
  // Initialization (asynchronous) [see control.js].
  // ```
  // @cb_ {function(err)} callack
  // ```
  init = function(cb_) {
    _super.init(cb_);
  };

  /****************************************************************************/
  /*                             PRIVATE HELPERS                              */
  /****************************************************************************/
  // ### push
  //
  // Pushes the current active page url to the control UI for eventual update 
  // (The url might not get directly updated if it is being edited, etc)
  push = function() {
    if(my.socket) {
      my.socket.emit('state', my.state);
    }
  };

  /****************************************************************************/
  /*                          SOCKET EVENT HANDLERS                           */
  /****************************************************************************/
  // ### socket_box_input
  //
  // Received when the user types into the box
  // ```
  // @input {string} the box input string
  // ```
  socket_box_input = function(input) {
    my.box_value = input;
  };
  
  // ### socket_box_input_submit
  //
  // Received whenever the box input is submitted by the user. We operate an 
  // heuristic here, if we detect that it is an url, we sanitize it and navigate
  // to it.
  //
  // Otherwise, we perform a google search
  // ```
  // @data {object} with `input` and `is_ctrl`
  // ```
  socket_box_input_submit = function(data) {
    var url_r = /^(http(s{0,1})\:\/\/){0,1}[a-z0-9\-\.]+(\.[a-z0-9]{2,4})+/;
    var ip_r = /^(http(s{0,1})\:\/\/){0,1}[0-9]{1,3}(\.[0-9]{1,3}){3}/
    var localhost_r = /^(http(s{0,1})\:\/\/){0,1}localhost+/
    var host_r = /^http(s{0,1})\:\/\/[a-z0-9\-\.]+/
    var http_r = /^http(s{0,1})\:\/\//;
    if(url_r.test(data.value) || 
       ip_r.test(data.value) || 
       localhost_r.test(data.value) || 
       host_r.test(data.value)) {
      if(!http_r.test(data.value)) {
        data.value = 'http://' + data.value;
      }
      my.session.frame().load_url(data.value);
    }
    else {
      var search_url = 'https://www.google.com/search?' +
      'q=' + escape(data.value) + '&' +
        'ie=UTF-8';
      my.session.frame().load_url(search_url);
    }
    push();
  };

  // ### socket_box_input_out
  //
  // Event triggered when the focus of the input box has been lost.
  socket_box_input_out = function() {
    my.session.frame().focus();
  };

  // ### socket_box_back
  //
  // Received when the back button is clicked
  socket_box_back = function() {
    my.session.frame().go_back_or_forward(-1);
  };

  // ### socket_box_forward
  //
  // Received when the back button is clicked
  socket_box_forward = function() {
    my.session.frame().go_back_or_forward(1);
  };


  /****************************************************************************/
  /*                              PUBLIC METHODS                              */
  /****************************************************************************/

  common.method(that, 'init', init, _super);
  common.method(that, 'handshake', handshake, _super);
  common.method(that, 'dimension', dimension, _super);

  return that;
};

exports.box = box;

