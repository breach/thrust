/*
 * Breach: session.js
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
// ## session
//
// ```
// @spec { base_url }
// ```
//
var session = function(spec, my) {
  var _super = {};
  my = my || {};
  spec = spec || {};

  my.base_url = spec.base_url;
  my.name = 'no_session';

  my.stack = { 
    socket: null,
    frame: null
  };
  my.home = { 
    socket: null,
    frame: null
  } 
  my.exo_browser = null;

  //
  // #### _public_
  //
  var handshake;   /* handshake(name, socket); */

  //
  // #### _private_
  //
  var init;        /* init(); */
  var init_stack;  /* init_stack(); */
  var init_home;   /* init_home(); */

  //
  // #### _that_
  //
  var that = {};

  //
  // ### handshake
  //
  // Receives the socket io socket associated with one of this session's frame.
  // (stack, home, ...)
  //
  // ```
  // @name   {string} the name of the frame
  // @socket {socket.io socket}
  // ```
  //
  handshake = function(name, socket) {
    var name_r = /^(br-[0-9]+)_(stack|home)$/;
    var name_m = name_r.exec(name);
    if(name_m) {
      console.log('HANDSHAKE: ' + my.name + ' ' + name_m[2]);
      my[name_m[2]].socket = socket;
      if(name_m[2] === 'stack')
        init_stack();
      if(name_m[2] === 'home')
        init_home();
    }
  };

  // 
  // ### init_stack
  //
  // Sets up all the socket bindings for the stack. This must be called after
  // the handshake.
  //
  //
  init_stack = function() {
    my.exo_browser.set_control_dimension(api.LEFT_CONTROL, 100);
  };

  //
  // ### init_home
  //
  // Sets up all the socket bindings for the home. This must be called after
  // the handshake.
  //
  init_home = function() {
  };

  //
  // ### init
  // 
  // Initialializes this session and spawns the associated exo_browser
  //
  init = function() {
    my.exo_browser = api.exo_browser({});
    my.name = my.exo_browser.name();

    my.stack.frame = api.exo_frame({
      name: my.name + '_stack',
      url: spec.base_url + '/stack/#/?session=' + my.name + '_stack'
    });
    my.home.frame = api.exo_frame({
      name: my.name + '_home',
      url: spec.base_url + '/home/#/?session=' + my.name + '_home'
    });

    my.exo_browser.set_control(api.LEFT_CONTROL, my.stack.frame);
    my.exo_browser.set_control_dimension(api.LEFT_CONTROL, 0);

    my.exo_browser.add_page(my.home.frame, function() {
      my.exo_browser.show_page(my.home.frame);
    });
  };
  
  init();

  common.method(that, 'handshake', handshake, _super);

  common.getter(that, 'name', my, 'name');

  return that;
};

exports.session = session;
