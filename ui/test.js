/*
 * Breach: test_internal.js
 *
 * (c) Copyright Stanislas Polu 2013. All rights reserved.
 *
 * @author: spolu
 *
 * @log:
 * 2013-08-11 spolu   Creation
 */

var express = require('express');
var util = require('util');
var http = require('http');

var api = require('./lib/api.js');

var b = api.exo_browser({
  size: [1000, 600]
});

var app = express();

var setup = function() {
  app.configure(function() {
    app.use('/', express.static(__dirname + '/static'));
    app.use(express.bodyParser());
    app.use(express.methodOverride());
    app.use(app.router);
  });
};

setup();

var http_srv = http.createServer(app).listen(8989);
console.error('Listening on port 8989');

b.set_control(api.LEFT_CONTROL, api.exo_frame({
  url: 'http://localhost:8989'
}));
b.set_control_dimension(api.LEFT_CONTROL, 300);

var p1 = api.exo_frame({
  url: 'http://localhost'
});
var p2 = api.exo_frame({
  url: 'http://localhost:8989'
});

b.add_page(p1);
b.add_page(p2);

var i = 0;
setInterval(function() {
  b.show_page((++i % 2) ? p1 : p2, function() {});
}, 500);


