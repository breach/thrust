/*
 * Breach: box_d.js
 *
 * Copyright (c) Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-16 spolu    Creation
 */
'use strict'

//
// ### StackCtrl
// `box` directive controller
//
angular.module('breach.directives').controller('BoxCtrl',
  function($scope, $element, $window, _socket) {
    $scope.$watch('active_url', function(active_url) {
      $scope.input = active_url;
      console.log('BOX_D ACTIVE URL: ' + active_url);
    });

    $scope.$watch('input', function(input) {
      _socket.emit('box_input', input);
    });

    $scope.submit = function() {
      _socket.emit('box_submit', $scope.input);
    };

    $scope.back = function() {
      _socket.emit('box_back');
    };
    $scope.forward = function() {
      _socket.emit('box_forward');
    };
  });

//
// ## box
//
// Directive representing the top box
//
// ```
// @=active_url    {string} the current active url
// ```
//
angular.module('breach.directives').directive('box', function() {
  return {
    restrict: 'E',
    replace: true,
    scope: {
      active_url: '=active_url',
    },
    templateUrl: 'partials/box_d.html',
    controller: 'BoxCtrl'
  };
});
