/**
 * exo_browser: common.js
 *
 * Copyright (c) 2013, Stanislas Polu. All rights reserved.
 * (see LICENSE file)
 *
 * @author: spolu
 *
 * @log:
 * - 2013-09-20 spolu   Creation
 */
var util = require('util');
var events = require('events');

"use strict";

/******************************************************************************/
/*                                CROCKFORD                                   */
/******************************************************************************/

//
// ### method
// ```
// @that {object} object to extend
// @name {string} the method name
// @method {function} the method
// @_super {object} parent object for functional inheritence
// ```
// Adds a method to the current object denoted by that and preserves _super 
// implementation (see Crockford)
//
exports.method = function(that, name, method, _super) {
  if(_super) {
    var m = that[name];
    _super[name] = function() {
      return m.apply(that, arguments);
    };    
  }
  that[name] = method;    
};

//
// ### getter
// ```
// @that {object} object to extend
// @name {string} the getter name
// @obj {object} the object targeted by the getter
// @key {string} the key to get on obj
// ```
// Generates a getter on obj for key
// 
exports.getter = function(that, name, obj, prop) {
  var getter = function() {
    return obj[prop];
  };
  that[name] = getter;
};

//
// ### setter
// ```
// @that {object} object to extend
// @name {string} the getter name
// @obj {object} the object targeted by the getter
// @key {string} the key to get on obj
// ```
// Generates a getter on obj for key
// 
exports.setter = function(that, name, obj, prop) {
  var setter = function (arg) {
    obj[prop] = arg;
    return that;
  };  
  that['set' + name.substring(0, 1).toUpperCase() + name.substring(1)] = setter;
  that['set' + '_' + name] = setter;
};

//
// ### responds
// ```
// @that {object} object to test
// @name {string} the method/getter/setter name
// ```
// Tests wether the object responds to the given method name
//
exports.responds = function(that, name) {
    return (that[name] && typeof that[name] === 'function');
};


/******************************************************************************/
/*                                 HELPERS                                    */
/******************************************************************************/

//
// #### once
// ```
// @fn {function} function to call once
// ```
// Returns a function that will call the underlying function only once
// whether it is called once or multiple times 
//
exports.once = function(fn) {
  if(fn === void 0 || fn === null || typeof fn !== 'function')
    throw new TypeError();

  var done = false;
  return function() {    
    if(!done) {
      args = Array.prototype.slice.call(arguments);
      done = true;
      fn.apply(null, args);
    }
  };
};

//
// ### remove
// ```
// @that {array} the array to operate on
// @e {object} element to remove from the array
// ```
// Removes the element e from the Array, using the JS '===' equality
//
exports.remove = function(that, e) {
  "use strict";
  
  if(that === void 0 || that === null || !Array.isArray(that))
    throw new TypeError();
  
  for(var i = that.length - 1; i >= 0; i--)
    if(e === that[i]) that.splice(i, 1);        
};

