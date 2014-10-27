/*
    Script by Alex Duros
*/

var mdns            = require('mdns-js'),
    Client          = require('castv2').Client,
    name            = {},
    express         = require('express'),
    app             = express(),
    io              = require('socket.io-client'),
    IP_CHROMECAST   = '', // '192.168.0.137',
    host            = '',
    sessionId       = '',
    connection      = null,
    heartbeat       = null,
    receiver        = null,
    socket          = null,
    reconnect       = false;


name.client = new Client();

var server = app.listen(8080, function() {
  // console.log('[Info] Listening on port %d', server.address().port);
  // console.log('[Info] Server is up');
  //IP_CHROMECAST = process.argv[2];
});

app.get('/discover', function(req, res) {
  // if(IP_CHROMECAST) {
    discoverChromecast(IP_CHROMECAST);
    res.status(202).json('Discovering');
  // }
  // else {
  //   console.log('[Error] IP Chromecast missing');
  //   res.status(409).json({ error: 'IP Chromecast missing' });
  // }
});

app.get('/cast', function(req, res) {
  if(host) {
    launchR7(host);
    openSocket();
    res.status(202).send({ message: 'Casting' });
    //console.log('01 - Casting ..')
  }
  else {
    console.log('00 - [Error] Discovering before casting');
    res.status(409).send({ error: 'Discovering before casting' });
  }
});

app.get('/key/:id', function(req, res) {
  var m = {
    action: 'key',
    params: parseInt(req.params.id, 10)
  };
  // console.log(m);
  if(socket) {
    socket.emit('message', m);
    //console.log(socket);
    res.status(202).send({ data: m });
  }
  else {
    console.log('10 - [Error] Opening socket before pushing any key');
    res.status(409).send({ error: 'Opening socket before pushing any key' });
  }
});

app.get('/zap/:channel', function(req, res) {
  var m = {
    action: 'zap',
    params: { channel: parseInt(req.params.channel, 10) }
  };
  // console.log(m);
  if(socket) {
    socket.emit('message', m);
    console.log("xx - zap on channel " + m);
    res.status(202).send({ data: m });
  }
  else {
    console.log('10 - [Error] Opening socket before pushing any key');
    res.status(409).send({ error: 'Opening socket before pushing any key' });
  }
});

app.get('/stop', function(req, res) {
  if(receiver && sessionId) {
    receiver.send({ type: 'STOP', sessionId: sessionId, requestId: 2 });
    connection.send({ type: 'CLOSE' });
    clearInterval(heartPulse);
    //socket.disconnect();
    sessionId = '';

    delete name.client;
    name.client = new Client();

    res.status(202).send({ message: 'Closing' });
    console.log('00 - Closing');
    discoverChromecast(IP_CHROMECAST);
  }
  else {
    console.log('00 - [Error] Connecting to Chromecast before closing it');
    res.status(409).send({ error: 'Connecting to Chromecast before closing it' });
  }
});

var discoverChromecast = function(ip) {
  var browser = mdns.createBrowser(mdns.tcp('googlecast'));

  var probe;

  browser.on('ready', function () {
    // console.log('[Info] Browser is ready');
    // The server will look for devices as long as it doesn't find one
    browser.discover();

    probe = setInterval(function() {
      console.log("00 - Discovering ..");
      browser.discover();
    }, 1000);
  });

  browser.on('update', function(service) {
    console.log('10 - Found device "%s" at %s:%d', service.type[0].name, service.addresses[0], service.port);
    //console.log(service)
    //if(service.addresses[0] === ip) {
      host = service.addresses[0];
      browser.stop();
    //}

    clearInterval(probe);
  });
};

var heartPulse;

var launchR7 = function(host) {
  name.client.connect(host, function() {
    connection = name.client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.tp.connection', 'JSON');
    heartbeat  = name.client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.tp.heartbeat', 'JSON');
    receiver   = name.client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.receiver', 'JSON');

    connection.send({ type: 'CONNECT' });

    heartPulse = setInterval(function() {
      heartbeat.send({ type: 'PING' });
    }, 5000);

    receiver.send({ type: 'LAUNCH', appId: 'A1AFC813', requestId: 1 });

    console.log('xx - Launch request sent')

    receiver.on('message', function(data, broadcast) {
      if(data.type === 'RECEIVER_STATUS' && sessionId === '') {
        if(data.status.applications) {
          sessionId = data.status.applications[0].sessionId;
        }
      }
    });

    // console.log('[Info] R7 launched');
  });
};

var openSocket = function() {
  console.log('xx - socket connection thrown to http://tv.canallabs.fr/?device=chrome&uid=xyz');

  if(reconnect) {
    
  }
  // socket = io.connect('http://tv.canallabs.fr/?device=chrome&uid=xyz', { path: '/remote/socket.io/' });

  socket = io.connect('http://localhost:3000/?device=chrome&uid=xyz', { path: '/remote/socket.io/' });

  socket.on('connect', function() {
    console.log('x1 - [Info] Socket connected');
  });

  socket.on('disconnect', function() {
    console.log('x0 - [Info] Socket disconnected');
  });

  socket.on('close', function() {
    console.log('x0 - [Info] Socket closed');
  });
}


discoverChromecast(IP_CHROMECAST);
