/*
 * Breach: breach.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-08 spolu   Creation
 */

/* Module dependencies */
var express = require('express');
var util = require('util');
var fs = require('fs');
var http = require('http');

_breach = apiDispatcher.requireBreach();

var _frames = [];
var _browsers = [];

console.error('foo');
_breach._createExoBrowser({
  size: [1000, 600]
}, function(b) { 
  _browsers.push(b);

  _breach._createExoFrame({ 
    name: 'test', 
    url: 'http://localhost:8989' 
  }, function(f) { 
    _frames.push(f);

    b._setControl(3, f, function() {});
    b._setControlDimension(3, 300, function() {});
  }); 



  _breach._createExoFrame({ 
    name: 'page1', 
    url: 'http://localhost:8989' 
  }, function(f) { 
    _frames.push(f);

    b._addPage(f, function() {
      b._showPage('page1', function() {});
    });
  }); 



  b._setOpenURLCallback(function() {
    console.error('OPEN_URL_CALLBACK');
  });
  b._setFrameLoadingStateChangeCallback(function(frame, loading) {
    console.error('FRAME_LOADING_STATE_CHANGE: ' + frame + ' ' + loading);
  });
});

var app = express();

var setup = function() {
  app.configure(function() {
    app.use('/', express.static(__dirname + '/ui'));
    app.use(express.bodyParser());
    app.use(express.methodOverride());
    app.use(app.router);
  });
};

setup();

var http_srv = http.createServer(app).listen(8989);
console.error('Listening on port 8989');
