/*
 * Breach: stack_d.js
 *
 * Copyright (c) Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-15 spolu    Creation
 */
'use strict'

//
// ### StackCtrl
// `stack` directive controller
//
angular.module('breach.directives').controller('StackCtrl',
  function($scope, _socket) {

    $scope.selection = 0;

    jQuery('body').bind('keyup.viewer', function(e) {
      $scope.$apply(function() {
        if(e.which === 74) { /* J */
          $scope.selection = ($scope.selection + 1) 
                        % $scope.entries.length;
        }
        if(e.which === 75) { /* K */
          $scope.selection = ($scope.selection - 1 + $scope.entries.length) 
                        % $scope.entries.length;
        }
        if(e.which === 13) { /* ENTER */
          _socket.emit('select_entry', 
                       $scope.entries[$scope.selection].name);
          $scope.selection = 0;
        }
      });
    });

    $scope.is_selected = function(entry) {
      return ($scope.entries[$scope.selection].name === entry.name);
    };

    $scope.$watch('entries', function(entries) {
      entries = entries || [];
      // console.log(JSON.stringify($scope.entries));
      
      entries.forEach(function(e, i) {
        if(e.navs.length > 0) {
          e.url = e.navs[0].url;
          e.title = e.navs[0].title || '';
          e.favicon = e.navs[0].favicon || '';
        }
        else {
          e.url = { hostname: '', href: '' };
          e.title = '';
          e.favicon = '';
        }
        e.active = (i === 0) ? true: false;
      });
    });

    $scope.select_entry = function(entry) {
      if(!entry.active)
        _socket.emit('select_entry', entry.name);
    };
  });

//
// ## stack
//
// Directive representing the actual stack
//
// ```
// @=entries    {array} the entries array
// ```
//
angular.module('breach.directives').directive('stack', function() {
  return {
    restrict: 'E',
    replace: true,
    scope: {
      entries: '=entries',
    },
    templateUrl: 'partials/stack_d.html',
    controller: 'StackCtrl'
  };
});
