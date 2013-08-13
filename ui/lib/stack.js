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
// @spec { session }
// ```
//
var stack = function(spec, my) {
  var _super = {};
  my = my || {};
  spec = spec || {};

  /* [{ frame, navs: [{ url, last, title }] }] */
  my.entries = [];

  //
  // ### _public_
  //
  var init;       /* init(cb_); */
  var handshake;  /* handshake(); */

  var new_entry;  /* new_entry([url]); */

  //
  // ### _private_
  //
  var entry_for_frame;    /* entry_for_frame(frame); */
  var push;               /* push(); */

  var frame_navigate;     /* frame_navigate(frame, url); */
  var frame_title_update  /* frame_title_updated(frame, title); */
  
  //
  // ### _protected_
  //
  var dimension;  /* dimension(); */

  //
  // #### _that_
  //
  var that = require('./control.js').control({
    session: spec.session,
    type: 'stack',
    control_type: api.LEFT_CONTROL
  }, my);

  /****************************************************************************/
  /*                            CONTROL INTERFACE                             */
  /****************************************************************************/
  //
  // ### dimension
  //  
  // Returns the desired canonical dimension
  // 
  dimension = function() {
    return 250;
  };

  // 
  // ### handshake
  //
  // Receives the socket and sets up events
  //
  // ```
  // @socket {socket.io socket}
  // ```
  //
  handshake = function(socket) {
    _super.handshake(socket);
    new_entry();
  };

  //
  // ### init
  // 
  // Initialization (asynchronous) [see control.js]. Also sets up the event
  // handlers on the exo_browser events
  // 
  // ```
  // @cb_ {function(err)} callack
  // ```
  //
  init = function(cb_) {
    _super.init(cb_);

    my.session.exo_browser().on('frame_navigate', frame_navigate);
    my.session.exo_browser().on('frame_title_updated', frame_title_updated);
  };

  /****************************************************************************/
  /*                             PRIVATE HELPERS                              */
  /****************************************************************************/
  //
  // ### entry_for_frame
  //
  // Retrieves the entry associated with this frame if it exists within this
  // stack or null otherwise
  // 
  // ```
  // @frame {exo_frame} the frame to search for
  // ```
  //
  entry_for_frame = function(frame) {
    for(var i = 0; i < my.entries.length; i ++) {
      if(my.entries[i].frame === frame)
        return my.entries[i];
    }
    return null;
  };

  //
  // ### push
  //
  // Pushes the entries to the control ui for update
  // 
  push = function() {
    var update = [];
    my.entries.forEach(function(e) {
      update.push({ navs: e.navs })
    });
    my.socket.emit('entries', update);
  };
  

  /****************************************************************************/
  /*                            EXOBROWSER EVENTS                             */
  /****************************************************************************/
  //
  // ### frame_navigate
  //
  // ExoBrowser event handler to update internal state of stack
  //
  // ```
  // @frame {exo_frame} the target frame
  // @url   {string} the new url
  // ```
  //
  frame_navigate = function(frame, url) {
    var e = entry_for_frame(frame);
    if(e) {
      console.log('NAVIGATE: ' + url);
      var i = 0;
      var exists = false;
      for(var i = e.navs.length - 1; i >= 0; i--) {
        if(e.navs[i].url === url) {
          var nav = e.navs.splice(i, 1)[0];
          nav.last = new Date();
          e.navs.push(nav);
          exists = true;
          break;
        }
      }
      if(!exists) {
        e.navs.push({
          url: url,
          title: '',
          last: new Date()
        });
      }
      push();
    }
  };

  //
  // ### frame_title_updated
  //
  // ExoBrowser event handler to update internal state of stack
  //
  // ```
  // @frame {exo_frame} the target frame
  // @title {string} the new title
  // ```
  //
  frame_title_updated = function(frame, title) {
    var e = entry_for_frame(frame);
    if(e) {
      console.log('TITLE: ' + url);
      e.title = title;
      push();
    }
  };

  /****************************************************************************/
  /*                              PUBLIC METHODS                              */
  /****************************************************************************/
  //
  // ### new_entry
  //
  // Creates a new entry for the provided url or a default one if not specified.
  //
  // ```
  // @url {string} the url to navigate to
  // ```
  //
  new_entry = function(url) {
    url = url || 'http://google.com';

    var e = {
      frame: api.exo_frame({
        url: url
      }),
      navs: []
    };

    my.entries.push(e);
    my.session.exo_browser().add_page(e.frame, function() {
      my.session.exo_browser().show_page(e.frame);
    });
    push();
  };
  

  common.method(that, 'init', init, _super);
  common.method(that, 'handshake', handshake, _super);
  common.method(that, 'dimension', dimension, _super);
  
  return that;
};

exports.stack = stack;
