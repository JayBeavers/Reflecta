var app = require('http').createServer(),
  io = require('socket.io').listen(app),
  fs = require('fs');

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

app.listen(8088);

var direction = 0;
var connected = null;

io.sockets.on('connection', function (socket) {
  connected = socket;
  
  socket.on('disconnect', function() {
    connected = null;
  });
});

var util = require('util');
var Reflecta = require('../../reflecta.js');
var reflecta = new Reflecta("COM4");

reflecta.on('error', function(error) { console.log(error); });
reflecta.on('portError', function(err) { console.log(err); });
  
reflecta.on('message', function(message) { console.log(message); });

reflecta.on('ready', function() {

  reflecta.on('heartbeat', function(heartbeat) {

    var hbData = {
      gyroscope: {
        x: heartbeat.data.readFloatBE(32),
        y: heartbeat.data.readFloatBE(28),
        z: heartbeat.data.readFloatBE(24)
      },
      accelerometer: {
        x: heartbeat.data.readFloatBE(20),
        y: heartbeat.data.readFloatBE(16),
        z: heartbeat.data.readFloatBE(12)
      },
      magnometer: {
        x: heartbeat.data.readFloatBE(8),
        y: heartbeat.data.readFloatBE(4),
        z: heartbeat.data.readFloatBE(0)
      }
    };
        
    console.log(heartbeat.collectingLoops + " : " + heartbeat.idleLoops + " : accel " + util.inspect(hbData.accelerometer) + ' : gyro ' + util.inspect(hbData.gyroscope) + ' : magno ' + util.inspect(hbData.magnometer));

    if (connected) {
      connected.emit('magnometer', { heading: hbData.magnometer.x % 360 });
    }
  });
});