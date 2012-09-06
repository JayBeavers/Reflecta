// Local settings, change to suit
var httpPort = 8088;
var ledPin = 11;

var app = require('http').createServer(),
  io = require('socket.io').listen(app),
  fs = require('fs'),
  util = require('util'),
  reflecta = require('../../reflecta.js');

// static webserver
app.on('request', function (req, res) {
  fs.readFile(__dirname + '/index.html',
    function (err, data) {
      if (err) {
        res.writeHead(500);
        return res.end('Error loading index.html');
      }

      res.writeHead(200);
      res.end(data);
    });
});

// Open a connection to the arduino and callback when port is opened

reflecta.detect(function(error, boards, ports) {

  if (error) {
    console.log(error);
    return;
  }

  var board = boards[0];

  board.on('error', function(error) { console.log("e: " + error); });
  board.on('warning', function(warning) { console.log("w: " + warning); });
  board.on('message', function(message) { console.log("m: " + message); });

  board.ARDU1.pinMode(ledPin, board.ARDU1.OUTPUT);

  // Wait until Arduino port is opened to listen for incoming connections  
  app.listen(httpPort);

  // Listen for incoming socketio messages
  io.sockets.on('connection', function (socket) {

    var ledState = 0;    
    socket.on('toggle', function() {
      console.log('Received (toggle)');
      ledState ^= 1; // XOR, this toggles ledState betwen values 0 (e.g. off) and 1 (e.g. on)
      board.ARDU1.digitalWrite(ledPin, ledState);
    });
  });
});