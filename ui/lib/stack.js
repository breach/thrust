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

  /* [{ frame, navs: [{ url, last, title, favicon }] }] */
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
  var entry_for_frame;      /* entry_for_frame(frame); */
  var entry_for_frame_name; /* entry_for_frame_name(frame); */
  var push;                 /* push(); */

  var frame_load_finish;    /* frame_load_finish(frame, url); */
  var frame_pending_url;    /* frame_pending_url(frame, url); */
  var frame_title_update;   /* frame_title_update(frame, title); */
  var frame_favicon_update; /* frame_favicon_update(frame, favicons); */

  var socket_select_entry;  /* socket_select_entry(name); */
  
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

    my.socket.on('select_entry', socket_select_entry);

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

    my.session.exo_browser().on('frame_favicon_update', frame_favicon_update);
    my.session.exo_browser().on('frame_pending_url', frame_pending_url);
    my.session.exo_browser().on('frame_load_finish', frame_load_finish);
    my.session.exo_browser().on('frame_title_update', frame_title_update);
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
  // ### entry_for_frame_name
  //
  // Retrieves the entry associated with this frame_name if it exists within 
  // this stack or null otherwise
  // 
  // ```
  // @frame {exo_frame} the frame to search for
  // ```
  //
  entry_for_frame_name = function(name) {
    for(var i = 0; i < my.entries.length; i ++) {
      if(my.entries[i].frame.name() === name)
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
      update.push({ name: e.frame.name(), navs: e.navs })
    });
    my.socket.emit('entries', update);
  };
  

  /****************************************************************************/
  /*                            EXOBROWSER EVENTS                             */
  /****************************************************************************/
  //
  // ### frame_load_finish
  //
  // We receive the final URL. We don't create a new nav we only update the
  // most recent one as it must have been preceded by a call to
  // `frame_pending_url`
  //
  // ```
  // @frame {exo_frame} the target frame
  // @url   {string} the new url
  // ```
  //
  frame_load_finish = function(frame, url) {
    var e = entry_for_frame(frame);
    if(e && e.navs.length > 0) {
      console.log('[STACK] FINAL: ' + url);
      e.navs[0].url = url;
      if(e.navs[0].title === 'Loading...') {
        e.navs[0].title = require('url').parse(url).hostname + ' - No TItLe';
      }
      e.navs[0].last = new Date();
      push();
    }
  };

  // ### frame_pending_url
  //
  // ExoBrowser event handler to update internal state of stack
  //
  // ```
  // @frame {exo_frame} the target frame
  // @url   {string} the new url
  // ```
  //
  frame_pending_url = function(frame, url) {
    var e = entry_for_frame(frame);
    if(e) {
      console.log('[STACK] PENDING: ' + url);
      var i = 0;
      var exists = false;
      for(var i = e.navs.length - 1; i >= 0; i--) {
        if(e.navs[i].url === url) {
          var nav = e.navs.splice(i, 1)[0];
          nav.last = new Date();
          e.navs.unshift(nav);
          exists = true;
          break;
        }
      }
      if(!exists) {
        e.navs.unshift({
          url: url,
          title: 'Loading...',
          last: new Date()
        });
      }
      push();
    }
  };

  //
  // ### frame_title_update
  //
  // ExoBrowser event handler to update internal state of stack
  //
  // ```
  // @frame {exo_frame} the target frame
  // @title {string} the new title
  // ```
  //
  frame_title_update = function(frame, title) {
    var e = entry_for_frame(frame);
    if(e && e.navs.length > 0) {
      console.log('[STACK] TITLE: ' + title);
      e.navs[0].title = title;
      push();
    }
  };

  //
  // ### frame_favicon_update
  //
  // Received whenever a favicon url is retrieved
  //
  // ```
  // @frame    {exo_frame} the target frame
  // @favicons {array} array of candidates favicon urls
  // ```
  //
  frame_favicon_update = function(frame, favicons) {
    /* TODO(spolu): for now we take the frist one always. Add the type into */
    /* the API so that a better logic can be implemented here.              */
    if(favicons.length > 0) {
      console.log('[STACK] FAVICON: ' + favicons[0]);
      var e = entry_for_frame(frame);
      if(e && e.navs.length > 0) {
        e.navs[0].favicon = favicons[0];
        push();
      }
    }
  };

  /****************************************************************************/
  /*                          SOCKET EVENT HANDLERS                           */
  /****************************************************************************/
  //
  // ### socket_select_entry
  //
  // Received when an entry is selected from the UI
  // 
  // ```
  // @name {string} the frame name of the entry
  // ```
  //
  socket_select_entry = function(name) {
    for(var i = 0; i < my.entries.length; i ++) {
      if(my.entries[i].frame.name() === name) {
        var e = my.entries.splice(i, 1)[0];
        my.entries.unshift(e);
        my.session.exo_browser().show_page(e.frame);
        push();
        break;
      }
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

    my.entries.unshift(e);
    my.session.exo_browser().add_page(e.frame, function() {
      my.session.exo_browser().show_page(e.frame);
    });
    push();
  };
  

  common.method(that, 'init', init, _super);
  common.method(that, 'handshake', handshake, _super);
  common.method(that, 'dimension', dimension, _super);
  common.method(that, 'new_entry', new_entry, _super);
  
  return that;
};

exports.stack = stack;
