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
  size: [800, 600]
}, function(b) { 
  _browsers.push(b);

  _breach._createExoFrame({ 
    name: 'test', 
    url: 'http://localhost:8989' 
  }, function(f) { 
    _frames.push(f);

    b._setControl(1, f, function() {});
    b._setControlDimension(1, 100, function() {});
  }); 

  b._setOpenURLCallback(function() {
    console.error('OPEN_URL_CALLBACK');
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
