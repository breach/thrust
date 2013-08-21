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
                        % $scope.pages.length;
          if($scope.selection === 0) $scope.selection += 1;
        }
        if(e.which === 75) { /* K */
          $scope.selection = ($scope.selection - 1 + $scope.pages.length) 
                        % $scope.pages.length;
          if($scope.selection === 0) $scope.selection = $scope.pages.length - 1;
        }
        if(e.which === 13) { /* ENTER */
          _socket.emit('select_page', 
                       $scope.pages[$scope.selection].name);
          $scope.selection = 0;
        }
      });
    });

    $scope.is_selected = function(page) {
      return ($scope.pages[$scope.selection].name === page.name);
    };

    $scope.$watch('pages', function(pages) {
      pages = pages || [];
      //console.log(JSON.stringify($scope.pages));
      
      pages.forEach(function(p, i) {
        if(p.state.entries.length > 0) {
          p.state.entries.forEach(function(n) {
            if(n.visible) {
              p.url = n.url;
              p.title = n.title;
              p.favicon = n.favicon;
            }
          });
        }
        p.url = p.url || { hostname: '', href: '' };
        p.title = p.title || '';
        p.favicon = p.favicon || ''
        p.active = (i === 0) ? true: false;
      });
    });

    $scope.select_page = function(page) {
      if(!page.active)
        _socket.emit('select_page', page.name);
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
      pages: '=pages',
    },
    templateUrl: 'partials/stack_d.html',
    controller: 'StackCtrl'
  };
});
