/*
 * ExoBrowser: socket_s.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-12 spolu   Creation
 */

'use strict';

angular.module('exo_browser.services').
  factory('_socket', function($rootScope) {
  var socket = io.connect();
  return {
    'on': function (event, callback) {
      socket.on(event, function () {  
        var args = arguments;
        $rootScope.$apply(function () {
          callback.apply(socket, args);
        });
      });
    },
    emit: function (event, data, callback) {
      socket.emit(event, data, function () {
        var args = arguments;
        $rootScope.$apply(function () {
          if (callback) {
            callback.apply(socket, args);
          }
        });
      })
    }
  };
});
