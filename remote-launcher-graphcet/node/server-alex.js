var app = require('express')();
app.use(function(req, res, next) {
	res.header("Access-Control-Allow-Origin", "*");
        res.header("Access-Control-Allow-Headers", "X-Requested-With");
        res.header("Access-Control-Allow-Headers", "Content-Type");
        res.header("Access-Control-Allow-Methods", "PUT, GET, POST, DELETE, OPTIONS");
        next();
});
 
var http = require('http').Server(app);
var io = require('socket.io')(http);
 
var users = {};
 
function mapUsers(users) {
  return Object.keys(users).map(function(i) {
    return {
      device: users[i].device,
      id:     i
    };
  });
}
 
function not(users, id) {
  var ret = [], ids = Object.keys(users);
  for(var i=0; i<ids.length; i++) {
    if (id !== ids[i]) {
      ret.push(users[ids[i]].socket);
    }
  }
  return ret;
}
 
io.on('connection', function(socket){
  function notify() {
    console.log('emit current users on socket', uid);
    console.log(mapUsers(users[uid]));
    socket.emit('users', mapUsers(users[uid]));
  }
 
  var uid = socket.handshake.query['uid'];
  var device = socket.handshake.query['device'];
  console.log('client connected with uid', uid);
 
  if (!users[uid]) {
    users[uid] = {};
  }
 
  users[uid][socket.id] = {
     device: device,
     socket: socket
  };
 
  notify();
 
  socket.on('disconnect', function() {
    console.log('client', socket.id, 'disconnected on', device);
    delete users[uid][socket.id];
    notify();
  });
 
  socket.on('message', function(data) {
    console.log('receive', data, 'from', socket.id);
    not(users[uid], socket.id).forEach(function(s) {
      s.emit('message', {data: data, source: socket.id});
    });
  })
});
 
http.listen(3000, function(){
  console.log('listening on *:3000');
});