var mdns            = require('mdns-js'),
    Client          = require('castv2').Client,
    client          = new Client(),
    express         = require('express'),
    app             = express(),
    io              = require('socket.io-client'),
    IP_CHROMECAST   = '',
    host            = '',
    sessionId       = '',
    connection      = null,
    heartbeat       = null,
    receiver        = null,
    socket          = null;

var server = app.listen(8080, function() {
  console.log('[Info] Listening on port %d', server.address().port);
  console.log('[Info] Server is up');
  IP_CHROMECAST = process.argv[2];
});

app.get('/discover', function(req, res) {
  if(IP_CHROMECAST) {
    discoverChromecast(IP_CHROMECAST);
    res.status(202).send('Discovering');
  }
  else {
    console.log('[Error] IP Chromecast missing');
    res.status(409).send({ error: 'IP Chromecast missing' });
  }
});

app.get('/cast', function(req, res) {
  if(host) {
    launchR7(host);
    openSocket();
    res.status(202).send({ message: 'Casting' });
  }
  else {
    console.log('[Error] Discovering before casting');
    res.status(409).send({ error: 'Discovering before casting' });
  }
});

app.get('/key/:id', function(req, res) {
  if(socket) {
    var m = {
      action: 'key',
      params: parseInt(req.params.id, 10)
    };

    socket.emit('message', m);
    console.log('sendMessage', m);
    res.status(202).send({ data: m });
  }
  else {
    console.log('[Error] Opening socket before pushing any key');
    res.status(409).send({ error: 'Opening socket before pushing any key' });
  }
});

app.get('/stop', function(req, res) {
  if(receiver && sessionId) {
    receiver.send({ type: 'STOP', sessionId: sessionId, requestId: 2 });
    connection.send({ type: 'CLOSE' });
    socket.disconnect();
    sessionId = '';
    res.status(202).send({ message: 'Closing' });
  }
  else {
    console.log('[Error] Connecting to Chromecast before closing it');
    res.status(409).send({ error: 'Connecting to Chromecast before closing it' });
  }
});

var discoverChromecast = function(ip) {
  var browser = mdns.createBrowser(mdns.tcp('googlecast'));

  browser.on('ready', function () {
    console.log('[Info] Browser is ready');
    browser.discover();
  });

  browser.on('update', function(service) {
    console.log('Found device "%s" at %s:%d', service.type[0].name, service.addresses[0], service.port);
    if(service.addresses[0] === ip) {
      host = service.addresses[0];
      browser.stop();
    }
  });
};

var launchR7 = function(host) {
  client.connect(host, function() {
    connection = client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.tp.connection', 'JSON');
    heartbeat  = client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.tp.heartbeat', 'JSON');
    receiver   = client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.receiver', 'JSON');

    connection.send({ type: 'CONNECT' });

    setInterval(function() {
      heartbeat.send({ type: 'PING' });
    }, 5000);

    receiver.send({ type: 'LAUNCH', appId: 'A1AFC813', requestId: 1 });

    receiver.on('message', function(data, broadcast) {
      if(data.type === 'RECEIVER_STATUS' && sessionId === '') {
        if(data.status.applications) {
          sessionId = data.status.applications[0].sessionId;
        }
      }
    });

    console.log('[Info] R7 launched');
  });
};

var openSocket = function() {
  socket = io.connect('http://tv.canallabs.fr/?device=chrome&uid=xyz', { path: '/remote/socket.io/' });

  socket.on('connect', function() {
    console.log('[Info] Socket connected');
  });

  socket.on('disconnect', function() {
    console.log('[Info] Socket disconnected');
  });

  socket.on('close', function() {
    console.log('[Info] Socket closed');
  });
}

