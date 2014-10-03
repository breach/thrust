var net = require('net');

var action_id = 10;
var BOUNDARY = "--(Foo)++__EXO_SHELL_BOUNDARY__++(Bar)--";
var ACTIONS = {};
var ACC = '';

var client = net.connect({ path: '/tmp/_exo_shell.sock' }, function() {
  console.log('Connected');
  main();
});

var perform = function(action, cb_) {
  ACTIONS[action._id.toString()] = cb_;
  client.write(JSON.stringify(action) + "\n" + BOUNDARY);
};

client.on('data', function(data) {
  ACC += data;
  var splits = ACC.split(BOUNDARY);
  if(splits.length > 0) {
    var data = splits.shift();
    ACC = splits.join(BOUNDARY);
    if(data.length > 0) {
      var action = JSON.parse(data);
      if(action._action === 'reply' && ACTIONS[action._id.toString()]) {
        var cb_ = ACTIONS[action._id.toString()];
        delete ACTIONS[action._id.toString()];
        if(action._error) {
          return cb_(new Error(action._error));
        }
        return cb_(null, action._result);
      }
    }
  }
});

var main = function() {

  perform({
    _id: ++action_id,
    _action: "create",
    _type: "session",
    _args: {
      off_the_record: false
    }
  }, function(err, res) {
    console.log('SESSION: ' + JSON.stringify(res));

    perform({
      _id: ++action_id,
      _action: "call",
      _target: res._target,
      _method: "devtools_url",
      _args: {}
    }, function(err, res) {
      console.log('DEVTOOLS: ' + JSON.stringify(res));
    });

    perform({
      _id: ++action_id,
      _action: "create",
      _type: "shell",
      _args: {
        root_url: "file:///" + __dirname + "/test.html",
        title: "ExoShell TEST 1",
        session_id: res._target,
        size: {
          width: 800,
          height: 700
        }
      }
    }, function(err, res) {
      console.log('SHELL: ' + JSON.stringify(res));

      perform({
        _id: ++action_id,
        _action: "call",
        _target: res._target,
        _method: "show",
        _args: {}
      }, function(err, res) {
        console.log('SHOW: ' + JSON.stringify(res));
      });
    });

  });

  perform({
    _id: ++action_id,
    _action: "create",
    _type: "session",
    _args: {
      off_the_record: false
    }
  }, function(err, res) {
    console.log('SESSION: ' + JSON.stringify(res));

    perform({
      _id: ++action_id,
      _action: "call",
      _target: res._target,
      _method: "devtools_url",
      _args: {}
    }, function(err, res) {
      console.log('DEVTOOLS: ' + JSON.stringify(res));
    });

    perform({
      _id: ++action_id,
      _action: "create",
      _type: "shell",
      _args: {
        root_url: "file:///" + __dirname + "/test.html",
        title: "ExoShell TEST 2",
        session_id: res._target,
        size: {
          width: 500,
          height: 400
        }
      }
    }, function(err, res) {
      console.log('SHELL: ' + JSON.stringify(res));

      perform({
        _id: ++action_id,
        _action: "call",
        _target: res._target,
        _method: "show",
        _args: {}
      }, function(err, res) {
        console.log('SHOW: ' + JSON.stringify(res));
      });
    });

  });
};

