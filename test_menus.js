var net = require('net');

var action_id = 0;
var BOUNDARY = "--(Foo)++__EXO_SHELL_BOUNDARY__++(Bar)--";

// var client = net.connect({ path: process.argv[2]+'.sock' }, function() {
//   console.log('Connected');


//   client.write(JSON.stringify(a) + "\n" + BOUNDARY);
// });



var TestClient = function (args, callback) {
  var api = {
    _socket: null,
    _stack: [],
    _queue: [],
    send: function (args, callback) {
      if (this._socket === null) {
        this._queue.push([args, callback]);
      }
      this._currentIndex += 1;
      args._id = this._currentIndex;
      this._stack[this._currentIndex] = callback;
      this._socket.write(JSON.stringify(args) + "\n" + api._boundary);
    },
    _handle: function (reply) {

      if (reply._id) this._stack[reply._id](null, reply);
      // Implement this obejct as an event emitterl
      //else this.emit('')
    },
    _boundary: BOUNDARY,
    _currentIndex: 0,
    _data: ''

  };

  var socket = net.connect({ path: process.argv[2]+'.sock' }, function() {
    api._socket = socket;
    api.send(args, callback);
    api._queue.forEach(function(item) {
      api.send(item[0], item[1]);
    })

    api._socket.on('data', function (buffer) {
      api._data += buffer;
      //console.log(buffer.toString());
      var splits = api._data.split(api._boundary);
      if(splits.length > 0) {
        var data = splits.shift();
        api._data = splits.join(BOUNDARY);
        if(data.length > 0) {
          var action = JSON.parse(data);
          api._handle(action);
        }
      }
    });
  })
  return api;
}

var a = {
  _action: "create",
  _type: "shell",
  _args: {
    root_url: "file:///" + __dirname + "/test.html",
    title: "ExoShell TEST",
    size: {
      width: 800,
      height: 700
    }
  }
};

var menuCreate = {
  _action: "create",
  _type: "menu",
  _args: {}
}
api = TestClient(a, function (err, action) {
  console.log('response from create');
  if(action._action === 'reply' && action._result && action._result._target) {
    var show = {
      _action: "call",
      _target: action._result._target,
      _method: "show",
      _args: {}
    };


    api.send(show, function (err, reply) {
      console.log('Reply from show');
      console.log(reply);
    });

    var move = {
      _action: "call",
      _target: action._result._target,
      _method: "move",
      _args: {
        x: 0,
        y: 0
      }
    };

    var resize = {
      _action: "call",
      _target: action._result._target,
      _method: "resize",
      _args: {
        width: 1024,
        height: 400
      }
    };

    setTimeout(function () {
      api.send(move, function (err, reply) {
        console.log('move::',reply);
      })
      api.send(resize, function(err, reply) {
        console.log('resize::', reply);
      })

      api.send(menuCreate, function(err, reply) {
        console.log('menu::create::', reply);
        var mainMenuId = reply._result._target;
        var setApplicationMenu = {
          _action: "call",
          _target: reply._result._target,
          _method: "setApplicationMenu", 
          _args: {}
        };
        var firstSubmenuId;
        api.send(menuCreate, function (err, reply2) {
          console.log('submenuCreate::', reply2);
          var id = firstSubmenuId = reply2._result._target;

          console.log("Main Menu ID", mainMenuId)
          console.log("Sub Menu ID", id)
          api.send(createMenuItem(0, id), function () {
            api.send(createSeperator(1, id), function () {
              api.send(createMenuItem(2, id), function() {
                api.send(insertSubMenuAt(0, mainMenuId, id),
                  function () {

                  })
              });
            });
          });
        })
        //Dont ever do what I am about to do.
        //Variable hoisting the first submenu id
        // and using settimeout to avoid callback hell here
        // is bad.
        setTimeout(function() {
        api.send(menuCreate, function (err, reply2) {
          console.log('submenuCreate::', reply2);
          var id = reply2._result._target;

          console.log("Main Menu ID", mainMenuId)
          console.log("Sub Menu ID", id)
          api.send(createMenuItem(0, id), function () {
            api.send(createSeperator(1, id), function () {
              api.send(createMenuItem(2, id), function() {
                api.send(insertSubMenuAt(1, firstSubmenuId, id),
                  function () {
                    api.send(setApplicationMenu, function() {
                      console.log(arguments);
                    })
                  })
              });
            });
          });
        })
        }, 1000);
      })
    }, 25);
  }
});

function insertSubMenuAt(index, targetId, subMenuId) {
  return {
    _action: "call",
    _target: targetId,
    _method: "insertSubMenuAt",
    _args: {
      index: index,
      command_id: 0,
      label: "SubMenu"+index,
      submenu_id: subMenuId
    }
  }
}

function createMenuItem(index, targetId, label) {
    var insertItemAt = {
      _action: "call",
      _target: targetId,
      _method: "insertItemAt",
      _args: {
        index: index,
        command_id:0,
        label: (label || "menuItem"+index)
      }
    }
    return insertItemAt;
}

function createSeperator(index, targetId) {
  var insertSeperatorAt = {
    _action: "call",
    _target: targetId,
    _method: "insertSeperatorAt",
    _args: {
      index: index
    }
  }
  return insertSeperatorAt;
}