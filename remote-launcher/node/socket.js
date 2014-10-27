'use strict';

var io = require('socket.io-client');

var socket = io.connect('http://tv.canallabs.fr/?device=Arduino&uid=xyz', { path: '/remote/socket.io/' });

socket.on('connect', function() {
	console.log('Socket connected');

	var m = {
	  action: 'key',
	  params: parseInt(process.argv[2], 10)
	};

	console.log('sendMessage', m);
	socket.emit('message', m);

	setTimeout(function() {
		socket.disconnect();
	  console.log('Exiting');
	  process.exit(0);
	}, 2000);
});

socket.on('disconnect', function() {
	console.log('Socket disconnected');
});