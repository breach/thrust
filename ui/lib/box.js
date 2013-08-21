/*
 * Breach: box.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-16 spolu   Creation
 */

var common = require('./common.js');
var factory = common.factory;
var api = require('./api.js');

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
    can_go_forward: false
  };

  //
  // ### _public_
  //
  var init;           /* init(cb_); */
  var handshake;      /* handshake(); */

  //
  // ### _private_
  //
  var push;                 /* push(); */

  var stack_active_page;    /* stack_active_page(page); */
  var socket_box_input;     /* socket_box_input(input); */
  var socket_box_submit;    /* socket_box_submit(input); */

  var socket_box_back;      /* socket_box_back(); */
  var socket_box_forward;   /* socket_box_forward(); */
  
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
    my.socket.on('box_submit', socket_box_submit);

    my.socket.on('box_back', socket_box_back);
    my.socket.on('box_forward', socket_box_forward);
    push();
  };

  // ### init
  // 
  // Initialization (asynchronous) [see control.js]. Also sets up the event
  // handlers on the stack control.
  // ```
  // @cb_ {function(err)} callack
  // ```
  init = function(cb_) {
    _super.init(cb_);

    my.session.stack().on('active_page', stack_active_page);
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
  /*                             STACK EVENTS                                 */
  /****************************************************************************/
  // ### stack_active_page
  //
  // Received from the stack whenever the active page is updated as it can
  // potentially impact the url to display
  // ```
  // @page {object} the current active page
  // ```
  stack_active_page = function(page) {
    page.state.entries.forEach(function(n) {
      if(n.visible) {
        my.state.value = n.url.href;
      }
    });
    my.state.can_go_back = page.state.can_go_back;
    my.state.can_go_forward = page.state.can_go_forward;
    if(page.box_value)
      my.state.value = page.box_value;
    push();
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
    var page = my.session.stack().active_page();
    if(page)
      page.box_value = input;
  };
  
  // ### socket_box_submit
  //
  // Received whenever the box input is submitted by the user. We operate an 
  // heuristic here, if we detect that it is an url, we sanitize it and navigate
  // to it.
  //
  // Otherwise, we perform a google search
  // ```
  // @input {string} the box input string
  // ```
  socket_box_submit = function(input) {
    var page = my.session.stack().active_page();
    if(page) {
      page.box_value = input;
      var url_r = /^(http(s{0,1})\:\/\/){0,1}[a-z0-9\-\.]+(\.[a-z0-9]{2,4})+/;
      var http_r = /^http(s{0,1})\:\/\//;
      if(url_r.test(input)) {
        if(!http_r.test(input)) {
          input = 'http://' + input;
        }
        page.frame.load_url(input);
      }
      else {
        var search_url = 'https://www.google.com/search?' +
                            'q=' + escape(input) + '&' +
                            'ie=UTF-8';
        page.frame.load_url(search_url);
      }
    }
  };

  // ### socket_box_back
  //
  // Received when the back button is clicked
  socket_box_back = function() {
    var page = my.session.stack().active_page();
    if(page) {
      page.frame.go_back_or_forward(-1);
    }
  };

  // ### socket_box_forward
  //
  // Received when the back button is clicked
  socket_box_forward = function() {
    var page = my.session.stack().active_page();
    if(page) {
      page.frame.go_back_or_forward(1);
    }
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

