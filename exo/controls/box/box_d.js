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
  function($scope, $element, $window, $timeout, _socket) {

    var _input = jQuery($element).find('input');

    $scope.$watch('state', function(state) {
      if(state) {
        $scope.can_go_back = state.can_go_back;
        $scope.can_go_forward = state.can_go_forward;
        $scope.stack_visible = state.stack_visible;
        if(!_input.is(':focus')) {
          $scope.value = state.value;
          $scope.last = $scope.value; 
        }
      }
    });


    _socket.on('select_all', function() {
      _input.focus().select();
    });
    
    _input.keydown(function() {
      if($scope.value !== $scope.last) {
        _socket.emit('box_input', $scope.value);
        $scope.last = $scope.value;
      }
    });

    /* TODO(spolu): Fix, CallStack Exceeded. Duh? */
    /*
    jQuery($element).find('input').focusout(function() {
      $(this).blur();
    });
    */
    
    $scope.submit = function() {
      _socket.emit('box_submit', $scope.value);
    };

    $scope.back = function() {
      _socket.emit('box_back');
    };
    $scope.forward = function() {
      _socket.emit('box_forward');
    };

    $scope.stack_toggle = function() {
      _socket.emit('stack_toggle');
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
      state: '=state',
    },
    templateUrl: 'box_d.html',
    controller: 'BoxCtrl'
  };
});
