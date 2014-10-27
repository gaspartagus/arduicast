'use strict';

var express  = require('express'),
    app      = express(),
    io       = require('socket.io-client');

var server = app.listen(8080, function() {
	console.log('Adress server = %s', server.address().adress);
  console.log('Listening on port %d', server.address().port);
});

var socket = io.connect('http://tv.canallabs.fr/?device=chrome&uid=xyz', { path: '/remote/socket.io/' });

socket.on('connect', function() {
	console.log('Socket connected');
});

socket.on('disconnect', function() {
	console.log('Socket disconnected');
});

socket.on('close', function() {
	console.log('Socket closed');
});

app.get('/key/:id', function(req, res){
	var m = {
	  action: 'key',
	  params: parseInt(req.params.id, 10)
	};

	console.log('sendMessage', m);
	socket.emit('message', m);
});

app.use(function(err, req, res, next){
  console.error(err.stack);
  res.send(404, 'Page introuvable');
});
