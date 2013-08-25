/*
 * Breach: keyboard_shortcuts.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-22 spolu   Creation
 */
var common = require('./common.js');
var factory = common.factory;
var api = require('./api.js');
var async = require('async');
var events = require('events');

// ## keyboard_shortcuts
//
// Handles keyboard events coming globally, perform some analysis (release
// order, modifier release), and emit shortcut events.
// 
// ```
// @spec { session }
// ```
var keyboard_shortcuts = function(spec, my) {
  var _super = {};
  my = my || {};
  spec = spec || {};

  my.session = spec.session;

  my.last = null;
  my.can_commit = false;

  //
  // #### _private_
  // 
  var is_last;  /* is_last(event); */

  //
  // #### that
  //
  var that = new events.EventEmitter();

  // ### is_last
  //
  // Computes whether it is the same event as last event
  // ```
  // @event {object} the event to compare to `last`
  // ```
  is_last = function(event) {
    if(my.last &&
       my.last.type === event.type &&
       my.last.modifiers === event.modifiers &&
       my.last.keycode === event.keycode) {
      return true;
    }
    return false;
  };


  // ### main event handler
  // 
  // Handles the session exo_browser `frame_keyboard` event
  //
  // Events: 
  // - `type`:
  //  `` RawKeyDown = 7 ``
  //  `` KeyUp      = 9 ``
  //
  // - `modifier`:
  //  `` ShiftKey   = 1 << 0 ``
  //  `` ControlKey = 1 << 1 ``
  //  `` AltKey     = 1 << 2 ``
  //  `` MetaKey    = 1 << 3 ``
  //  `` IsLeft     = 1 << 11 ``
  //  `` IsRight    = 1 << 12 ``
  // ```
  // @frame {exo_frame} source
  // @event {object} keyboard event
  // ```
  my.session.exo_browser().on('frame_keyboard', function(frame, event) {
    console.log(JSON.stringify(event));

    if(event.type === 7 && (event.modifiers & (1 << 1)) &&
       event.keycode === 84 && !is_last(event)) {
      /* Ctrl - T ; No Repetition */
      that.emit('new_page');
    }

    if(event.type === 7 && (event.modifiers & (1 << 1)) &&
       event.keycode === 71 && !is_last(event)) {
      /* Ctrl - G ; No Repetition */
      that.emit('go');
    }

    if(event.type === 7 && (event.modifiers & (1 << 1)) &&
       event.keycode === 72 && !is_last(event)) {
      /* Ctrl - H ; No Repetition */
      that.emit('back');
    }
    if(event.type === 7 && (event.modifiers & (1 << 1)) &&
       event.keycode === 76 && !is_last(event)) {
      /* Ctrl - L ; No Repetition */
      that.emit('forward');
    }

    if(event.type === 7 && (event.modifiers & (1 << 1)) &&
       event.keycode === 77 && !is_last(event)) {
      /* Ctrl - M ; No Repetition */
      that.emit('stack_toggle');
    }

    if(event.type === 7 && (event.modifiers === (1 << 1)) &&
       event.keycode === 74) {
      /* Ctrl - J ; Repetition OK */
      that.emit('stack_next');
      my.can_commit = true;
    }
    if(event.type === 7 && (event.modifiers === (1 << 1)) &&
       event.keycode === 75) {
      /* Ctrl - K ; Repetition OK */
      that.emit('stack_prev');
      my.can_commit = true;
    }
    if(event.type === 9 && (event.modifiers === (1 << 11)) &&
       event.keycode === 17) {
      /* Ctrl (Release); No Repetition */
      if(my.can_commit) {
        my.can_commit = false;
        that.emit('stack_commit');
      }
    }

    if(event.type === 7 && (event.modifiers === (1 << 0 | 1 << 1)) &&
       event.keycode === 74) {
      /* Ctrl - Shit - J ; Repetition OK */
      that.emit('stack_move_next');
    }
    if(event.type === 7 && (event.modifiers === (1 << 0 | 1 << 1)) && 
       event.keycode === 75) {
      /* Ctrl - Shit - K ; Repetition OK */
      that.emit('stack_move_prev');
    }

    if(event.type === 7 && (event.modifiers === (1 << 1)) && 
       event.keycode === 87 && !is_last(event)) {
      /* Ctrl - W ; No Repetition */
      that.emit('stack_close');
    }

    if(event.type === 7 && (event.modifiers === (1 << 1)) && 
       event.keycode === 80 && !is_last(event)) {
      /* Ctrl - W ; No Repetition */
      that.emit('stack_pin');
    }
      

    my.last = event;

    /*
    if(event.type === 9 &&
       event.modifiers === 2 &&
         event.keycode === 71) {
      toggle_stack(true);
    my.stack.focus();
    }
    if(event.type === 9 &&
       event.modifiers === 2 &&
         event.keycode === 72) {
      toggle_stack(false);
    }
    if(event.type === 9 &&
       event.modifiers === 2 &&
         event.keycode === 84) {
      my.stack.new_page();
    }
   */
  });

  return that;
};

exports.keyboard_shortcuts = keyboard_shortcuts;

