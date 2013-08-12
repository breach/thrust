/*
 * Nitrogram: app.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-12 spolu  Creation
 */

'use strict';

//
// ## App Module
//
angular.module('breach', ['breach.services', 
                          'breach.directives', 
                          'breach.filters']);

//
// ### StackTopCtrl
// Initializations goes here as well as global objects
//
function StackTopCtrl($scope, $location, $rootScope, $window, $timeout,
                      _session) {
  $scope.session = _session.name();
}

angular.module('breach.directives', []);
angular.module('breach.filters', []);
angular.module('breach.services', []);

