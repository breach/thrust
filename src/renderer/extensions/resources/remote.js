// Copyright (c) 2014 Stanislas Polu.
// See the LICENSE file.

var RemoteNatives = requireNative('remote_natives');

var remote = function(spec, my) {
  my = my || {};
  spec = spec || {};

  my.listeners = [];

  //
  // _public_
  //
  var send;    /* send(message); */
  var listen;  /* listen(cb_); */
  var remove;  /* remove(cb_); */

  //
  // _private_
  //
  var handler; /* handler(message); */

  //
  // #### _that_
  //
  var that = {};

  /****************************************************************************/
  /* PRIVATE HELPERS */
  /****************************************************************************/
  // ### handler
  // 
  // Handler for the RemoteNatives object
  // ```
  // @msg {object} the message to dispatch
  // ```
  handler = function(msg) {
    my.listeners.forEach(function(cb_) {
      return cb_(msg);
    });
  };

  /****************************************************************************/
  /* REMOTE API */
  /****************************************************************************/
  // ### send
  //
  // Sends a remote message
  // ```
  // @message {object} the message to send
  // ```
  send = function(message) {
    if(typeof message !== 'object') {
      message = { payload: message };
    }
    RemoteNatives.SendMessage(message);
  };

  // ### listen
  //
  // Adds a listener for remote messages
  // ```
  // @cb_ {function(message)} 
  // ```
  listen = function(cb_) {
    remove(cb_);
    my.listeners.push(cb_);
  };

  // ### remove
  //
  // Removes a message listener
  // ```
  // @cb_ {function(message)} 
  // ```
  remove = function(cb_) {
    for(var i = my.listeners.length - 1; i >= 0; i--) {
      if(my.listeners[i] === cb_) {
        my.listeners.splice(i, 1);
      }
    }
  };

  RemoteNatives.SetHandler(handler);

  that.send = send;
  that.listen = listen;
  that.remove = remove;

  return that;
};

/******************************************************************************/
/* API REGISTRATION */
/******************************************************************************/
window.THRUST = window.THRUST || {};
window.THRUST.remote = remote({});

// GOAL: window.remote.send({...});
//       window.remote.listen(function(msg) {...});
//       window.remote.remove(function(msg) {...});

/* TODO(spolu):
//       window.remote.emit('type', {...})
//       window.remote.on('type', function(evt) {...});
//       window.remote.expose('method', function(args, cb_) {...});
//       window.remote.call('method', args, function(err, res) {...});
*/
