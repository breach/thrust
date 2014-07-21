/*
 * ExoBrowser: box_d.js
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
angular.module('exo_browser.directives').controller('BoxCtrl',
  function($scope, $element, $window, $timeout, _socket) {

    var _input = jQuery($element).find('input');

    $scope.$watch('state', function(state) {
      if(state) {
        $scope.can_go_back = state.can_go_back;
        $scope.can_go_forward = state.can_go_forward;
        $scope.mode = state.mode;
        $scope.value = state.value;
        $scope.last = $scope.value; 
      }
    });


    _socket.on('select_all', function() {
      _input.focus().select();
    });

    $scope.$watch('value', function(value) {
      if($scope.value !== $scope.last) {
        _socket.emit('box_input', $scope.value);
        $scope.last = $scope.value;
      }
    });

    _input.keydown(function(evt) {
      if(evt.keyCode === 27) {
        _socket.emit('box_input_out');
      }
    });
    
    $scope.submit = function() {
      console.log('submit');
      _socket.emit('box_input_submit', { 
        value: $scope.value, 
        is_ctrl: false
      });
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
angular.module('exo_browser.directives').directive('box', function() {
  return {
    restrict: 'E',
    replace: true,
    scope: {
      state: '=state',
    },
    templateUrl: 'box_d.html',
    controller: 'BoxCtrl'
  };
});
