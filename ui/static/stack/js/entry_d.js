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
  function($scope) {
    $scope.$watch('data', function(entry) {
      console.log('EntryCtrl: ' + JSON.stringify(entry));
      if(entry.navs.length > 0) {
        $scope.title = entry.navs[0].title;
        $scope.url = entry.navs[0].url;
      }
    });
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
