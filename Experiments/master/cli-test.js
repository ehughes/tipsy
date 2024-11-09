var mdns = require('mdns');
var WebSocket = require('ws');
var sprintf = require('sprintf');
var Neos = require('./console/src/Neos');

var log = function(fmt, argv) {
  if (!argv) {
    console.log(fmt);
  }
  else if (Array.isArray(argv)) {
    console.log(sprintf.vsprintf(fmt, argv));
  }
  else {
    console.log(fmt + ' ' + JSON.stringify(argv));
  }
};

var neos = new Neos(log);

neos.browser({
  start: function(up) {
    // watch all http servers
    var browser = mdns.createBrowser(mdns.tcp('http'));
    browser.on('serviceUp', function(service) {
      up ({
        ip: service.addresses[0],
        port: service.port,
        name: service.name
      });
    });
    browser.start();
  }
});

neos.socket({
  connect: function (url, name, cb) {
    // connect to ws node
    var ws = new WebSocket(url);
    ws.on('open', function () {
      log("open %s", [url]);
      // setInterval(function() {
      //   ws.send(JSON.stringify({ name: 'test', blah: true}));
      // }, 1000);
    });
    ws.on('close', function () {
      log("close %s", [name]);
      cb(true, name);
    });
    ws.on('message', function(data, flags) {
      // flags.binary will be set if a binary data is received.
      // flags.masked will be set if the data was masked.
      cb(null, name, JSON.parse(data));
    });
    ws.on('error', function(error) {
      cb(error, name);
    });
    return ws;
  },
  send: function (ws, data) {
    ws.send(data);
  }
});

neos.start();
setInterval(() => {
  neos.tick();
}, 100);

var readline = require('readline');
var rl = readline.createInterface(process.stdin, process.stdout);
rl.setPrompt('guess> ');
rl.prompt();
rl.on('line', function(line) {
  log("HERE", line);
  neos.playGame({});
}).on('close',function(){
    process.exit(0);
});

