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

// ### stack
//
// ```
// @spec { session }
// ```
var stack = function(spec, my) {
  var _super = {};
  my = my || {};
  spec = spec || {};

  /* [{ frame,       */
  /*    state,       */
  /*    box_value }] */
  my.pages = [];
  my.favicons = {}

  //
  // ### _public_
  //
  var init;         /* init(cb_); */
  var handshake;    /* handshake(); */

  var new_page;     /* new_page([url]); */
  var active_page;  /* active_page(); */

  //
  // ### _private_
  //
  var page_for_frame;         /* page_for_frame(frame); */
  var page_for_frame_name;    /* page_for_frame_name(frame); */
  var push;                   /* push(); */

  var frame_navigation_state; /* frame_navigation_state(frame, state); */
  var frame_favicon_update;   /* frame_favicon_update(frame, favicons); */

  var socket_select_page;     /* socket_select_page(name); */

  var shortcut_new_page;      /* shortcut_new_page(); */
  
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
  // ### dimension
  //  
  // Returns the desired canonical dimension
  dimension = function() {
    return 250;
  };

  // ### handshake
  //
  // Receives the socket and sets up events
  // ```
  // @socket {socket.io socket}
  // ```
  handshake = function(socket) {
    _super.handshake(socket);

    my.socket.on('select_page', socket_select_page);

    new_page();
  };

  // ### init
  // 
  // Initialization (asynchronous) [see control.js]. Also sets up the event
  // handlers on the exo_browser events
  // ```
  // @cb_ {function(err)} callack
  // ```
  init = function(cb_) {
    _super.init(cb_);

    my.session.exo_browser().on('frame_navigation_state', 
                                frame_navigation_state);
    my.session.exo_browser().on('frame_favicon_update', 
                                frame_favicon_update);

    my.session.keyboard_shortcuts().on('new_page', shortcut_new_page);
  };


  /****************************************************************************/
  /*                             PRIVATE HELPERS                              */
  /****************************************************************************/
  // ### page_for_frame
  //
  // Retrieves the page associated with this frame if it exists within this
  // stack or null otherwise
  // ```
  // @frame {exo_frame} the frame to search for
  // ```
  page_for_frame = function(frame) {
    for(var i = 0; i < my.pages.length; i ++) {
      if(my.pages[i].frame === frame)
        return my.pages[i];
    }
    return null;
  };

  // ### page_for_frame_name
  //
  // Retrieves the page associated with this frame_name if it exists within 
  // this stack or null otherwise
  // ```
  // @frame {exo_frame} the frame to search for
  // ```
  page_for_frame_name = function(name) {
    for(var i = 0; i < my.pages.length; i ++) {
      if(my.pages[i].frame.name() === name)
        return my.pages[i];
    }
    return null;
  };

  // ### push
  //
  // Pushes the entries to the control ui for update
  push = function() {
    var update = [];
    my.pages.forEach(function(p) {
      update.push({ name: p.frame.name(), state: p.state })
    });
    my.socket.emit('pages', update);
    if(my.pages.length > 0) {
      that.emit('active_page', my.pages[0]);
    }
  };
  

  /****************************************************************************/
  /*                            EXOBROWSER EVENTS                             */
  /****************************************************************************/
  // ### frame_navigation_state
  //
  // An update has been made to the navigation state, so we should update our
  // own internal state
  // ```
  // @frame {exo_frame} the target_frame
  // @state {object} the navigation state
  // ```
  frame_navigation_state = function(frame, state) {
    var p = page_for_frame(frame);
    if(p) {
      /* We clear the box_value for this page only if the state visible entry */
      /* `id` has changed (we navigated somewhere)                            */
      var new_id = null, old_id = null;
      p.state.entries.forEach(function(n) { if(n.visible) old_id = n.id; });
      state.entries.forEach(function(n) { if(n.visible) new_id = n.id; });
      if(new_id !== old_id && new_id !== null) {
        p.box_value = null;
      }

      p.state = state;
      p.state.entries.forEach(function(n) {
        if(my.favicons[n.id]) {
          n.favicon = my.favicons[n.id];
        }
      });
      push();
    }
  };

  // ### frame_favicon_update
  //
  // We receive the favicon (and not use the `navigation_state` because of:
  // CRBUG 277069) and attempt to stitch it in the correct state entry
  // ```
  // @frame    {exo_frame} the target frame
  // @favicons {array} array of candidates favicon urls
  // ```
  frame_favicon_update = function(frame, favicons) {
    /* TODO(spolu): for now we take the frist one always. Add the type into */
    /* the API so that a better logic can be implemented here.              */
    var p = page_for_frame(frame);
    if(favicons.length > 0 && p) {
      p.state.entries.forEach(function(n) {
        if(n.visible) {
          my.favicons[n.id] = favicons[0];
          n.favicon = favicons[0];
        }
      });
      push();
    }
  }; 

  /****************************************************************************/
  /*                          SOCKET EVENT HANDLERS                           */
  /****************************************************************************/
  // ### socket_select_page
  //
  // Received when an page is selected from the UI
  // ```
  // @name {string} the frame name of the page
  // ```
  socket_select_page = function(name) {
    for(var i = 0; i < my.pages.length; i ++) {
      if(my.pages[i].frame.name() === name) {
        var p = my.pages.splice(i, 1)[0];
        my.pages.unshift(p);
        my.session.exo_browser().show_page(p.frame);
        push();
        break;
      }
    }
  };

  /****************************************************************************/
  /*                      KEYBOARD SHORTCUT EVENT HANDLERS                    */
  /****************************************************************************/
  // ### shortcut_new_page
  //
  // Keyboard shorcut to create a new page
  shortcut_new_page = function() {
    that.new_page();
  };

  /****************************************************************************/
  /*                              PUBLIC METHODS                              */
  /****************************************************************************/
  // ### new_page
  //
  // Creates a new page for the provided url or a default one if not specified.
  // The url is supposed to be a valid url. There's nothing smart here.
  // ```
  // @url        {string} the url to navigate to
  // ```
  //
  new_page = function(url) {
    var box_focus = !url ? true : false;
    url = url || (my.session.base_url() + '/home.html');

    var p = {
      frame: api.exo_frame({
        url: url
      }),
      state: { 
        entries: [],
        can_go_back: false,
        can_go_forward: false
      },
      box_value: url ? null : ''
    };

    my.pages.unshift(p);

    my.session.exo_browser().add_page(p.frame, function() {
      my.session.exo_browser().show_page(p.frame, function() {
        if(!box_focus) {
          setTimeout(function() {
            p.frame.focus();
          }, 100);
        }
        else {
          setTimeout(function() {
            my.session.box().focus();
          }, 100);
        }
      });
    });
    push();
  };

  // ### active_page
  //
  // Returns the current actrive page
  active_page = function() {
    if(my.pages.length > 0) {
      return my.pages[0]
    }
    return null;
  };
  

  common.method(that, 'init', init, _super);
  common.method(that, 'handshake', handshake, _super);
  common.method(that, 'dimension', dimension, _super);
  common.method(that, 'new_page', new_page, _super);
  common.method(that, 'active_page', active_page, _super);
  
  return that;
};

exports.stack = stack;
