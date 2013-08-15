/*
 * Breach: entry_d.js
 *
 * Copyright (c) Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-13 spolu    Creation
 */
'use strict'

//
// ### EntryCtrl
// `entry` directive controller
//
angular.module('breach.directives').controller('EntryCtrl',
  function($scope, _socket) {
    $scope.$watch('data', function(entry) {
      if(entry.navs.length > 0) {
        $scope.entry = entry;
        $scope.favicon = entry.navs[0].favicon;
        $scope.title = entry.navs[0].title;
        $scope.url = entry.navs[0].url;
      }
    });

    $scope.select = function() {
      _socket.emit('select_entry', $scope.entry.name);
    };
  });

//
// ## entry
//
// Menu Directive used for Insights and Galleries
// ```
// @=data    {object} the entry object
// ```
//
angular.module('breach.directives').directive('entry', function() {
  return {
    restrict: 'E',
    replace: true,
    scope: {
      data: '=data',
    },
    templateUrl: 'partials/entry_d.html',
    controller: 'EntryCtrl'
  };
});
