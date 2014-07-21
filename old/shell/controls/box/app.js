/*
 * ExoBrowser: app.js [box]
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-16 spolu  Creation
 */

'use strict';

//
// ## App Module
//
angular.module('exo_browser', ['exo_browser.services', 
                               'exo_browser.directives', 
                               'exo_browser.filters']);

//
// ###  BoxTopCtrl
// Initializations goes here as well as global objects
//
function BoxTopCtrl($scope, $location, $rootScope, $window, $timeout,
                    _session, _socket) {

  /* Handhsaking */
  _socket.emit('handshake', _session.name() + '_box');

  _socket.on('state', function(state) {
    $scope.state = state;
  });
}

angular.module('exo_browser.directives', []);
angular.module('exo_browser.filters', []);
angular.module('exo_browser.services', []);

