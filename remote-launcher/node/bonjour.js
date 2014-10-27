var mdns   = require('mdns-js');
var Client = require('castv2').Client;

var browser = mdns.createBrowser(mdns.tcp('googlecast'));

browser.on('ready', function () {
    console.log('browser is ready');
    browser.discover();
});

browser.on('update', function(service) {
  console.log('Found device "%s" at %s:%d', service.type[0].name, service.addresses[0], service.port);
  if(service.addresses[0] === '10.0.2.113') {
  // if(service.addresses[0] === '10.0.2.252') {
    onDeviceUp(service.addresses[0]);
    browser.stop();
  }
});

function onDeviceUp(host) {

  var client = new Client();
  var sessionId = '';
  var transportId = '';

  client.connect(host, function() {
    // create various namespace handlers
    var connection = client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.tp.connection', 'JSON');
    var heartbeat  = client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.tp.heartbeat', 'JSON');
    var receiver   = client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.google.cast.receiver', 'JSON');
    var canalG7    = client.createChannel('sender-0', 'receiver-0', 'urn:x-cast:com.canallabs.chromecast', 'JSON');

    // establish virtual connection to the receiver
    connection.send({ type: 'CONNECT' });

    // start heartbeating
    setInterval(function() {
      heartbeat.send({ type: 'PING' });
    }, 5000);

    // launch YouTube app
    receiver.send({ type: 'LAUNCH', appId: 'A1AFC813', requestId: 1 });

    // display receiver status updates
    receiver.on('message', function(data, broadcast) {
      if(data.type === 'CLOSE') {
        console.log('CLOSE');
      }
    });
  });

}