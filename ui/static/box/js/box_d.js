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
    $scope.$watch('state', function(state) {
      if(state) {
        $scope.can_go_back = state.can_go_back;
        $scope.can_go_forward = state.can_go_forward;
        $scope.value = state.value;
      }
    });


    _socket.on('select_all', function() {
      jQuery($element).find('input').focus().select();
    });
    
    jQuery($element).find('input').keydown(function() {
      _socket.emit('box_input', $scope.value);
    });
    jQuery($element).find('input').focusout(function() {
      $(this).blur();
    });

    
    $scope.submit = function() {
      _socket.emit('box_submit', $scope.value);
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
      state: '=state',
    },
    templateUrl: 'partials/box_d.html',
    controller: 'BoxCtrl'
  };
});
