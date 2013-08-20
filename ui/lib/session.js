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
var async = require('async');


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
  my.exo_browser = null;

  my.loading_frame = null;

  my.stack = null;
  my.box = null;

  //
  // #### _public_
  //
  var toggle_stack; /* toggle_stack([visible]); */
  var toggle_box;   /* toggle_box([visible]); */

  var handshake;   /* handshake(name, socket); */

  //
  // #### _private_
  //
  var init;        /* init(); */

  //
  // #### _that_
  //
  var that = {};

  //
  // ### toggle_stack
  //
  // If no argument is provided it just toggles the stack visibility. If a
  // visibility argument is provided, it shows or hides it.
  //
  // ```
  // @visibilty {boolean} toggle to this visibility
  // ```
  //
  toggle_stack = function(visible) {
    if(typeof visible === 'boolean') {
      if(visible) 
        my.stack.show();
      else
        my.stack.hide();
    }
    else {
      if(my.stack.visible())
        my.stack.hide();
      else
        my.stack.show();
    }
  };

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
    var name_r = /^(br-[0-9]+)_(stack|box)$/;
    var name_m = name_r.exec(name);
    if(name_m) {
      if(name_m[2] === 'stack')
        my.stack.handshake(socket);
      if(name_m[2] === 'box')
        my.box.handshake(socket);
    }
  };

  //
  // ### init
  // 
  // Initialializes this session and spawns the associated exo_browser
  //
  init = function() {
    my.exo_browser = api.exo_browser({
      size: [1200, 768]
    });
    my.name = my.exo_browser.name();

    my.loading_frame = api.exo_frame({
      name: my.name + '_loading',
      url: my.base_url + '/loading.html'
    });
    my.exo_browser.add_page(my.loading_frame, function() {
      my.exo_browser.show_page(my.loading_frame);
    });

    my.stack = require('./stack.js').stack({
      session: that
    });
    my.box = require('./box.js').box({
      session: that
    });

    async.parallel({
      stack: function(cb_) {
        my.stack.init(cb_);
      },
      box: function(cb_) {
        my.box.init(cb_);
      },
    }, function(err) {
      toggle_stack(true);
      my.box.show();
    });

    my.exo_browser.on('frame_keyboard', function(frame, event) { 
      //console.log(JSON.stringify(event));
      if(event.type === 9 &&
         event.modifiers === 2 &&
         event.keycode === 71) {
        toggle_stack(true);
        /* TODO(spolu): focus on stack */
      }
      if(event.type === 9 &&
         event.modifiers === 2 &&
         event.keycode === 72) {
        toggle_stack(false);
      }
      if(event.type === 9 &&
         event.modifiers === 2 &&
         event.keycode === 84) {
        my.stack.new_entry();
      }
    });
  };
  
  common.method(that, 'handshake', handshake, _super);
  common.method(that, 'toggle_stack', toggle_stack, _super);
  common.method(that, 'toggle_box', toggle_box, _super);

  common.getter(that, 'name', my, 'name');
  common.getter(that, 'exo_browser', my, 'exo_browser');
  common.getter(that, 'base_url', my, 'base_url');

  common.getter(that, 'stack', my, 'stack');
  common.getter(that, 'box', my, 'box');
  common.getter(that, 'base_url', my, 'base_url');

  init();

  return that;
};

exports.session = session;
