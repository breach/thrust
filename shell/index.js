/*
 * ExoBrowser: [shell] index.js
 *
 * (c) Copyright Stanislas Polu 2014. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2014-04-29 spolu   Move to express 4.0
 * 2013-09-06 spolu   Exp1 process.exit on session kill
 * 2013-08-11 spolu   Creation
 */
var express = require('express');
var http = require('http');
var common = require('./lib/common.js');

var factory = common.factory;
var app = express();


var sessions = {};

//
// ### init
//
factory.log().out('Starting...');
(function() {
  /* App Configuration */
  app.use('/', express.static(__dirname + '/controls'));
  app.use(require('body-parser')());
  app.use(require('method-override')())

  var http_srv = http.createServer(app).listen(8383, '127.0.0.1');
  console.error('HTTP Server started on `http://127.0.0.1:8383`');

  var io = require('socket.io').listen(http_srv, {
    'log level': 1
  });

  io.sockets.on('connection', function (socket) {
    socket.on('handshake', function (name) {
      var name_r = /^(br-[0-9]+)_(.*)$/;
      var name_m = name_r.exec(name);
      if(name_m && sessions[name_m[1]]) {
        sessions[name_m[1]].handshake(name, socket);
      }
    });
  });
})();

//
// ### bootstrap
//
(function() {
  var s = require('./lib/session.js').session({ 
    base_url: 'http://127.0.0.1:8383' 
  })
  sessions[s.name()] = s;
  s.on('kill', function() {
    delete sessions[s.name()];
    if(global.gc) global.gc();
    process.exit(0);
  });
})();

