var app = require('http').createServer(handler)
  , io = require('socket.io').listen(app)
  , fs = require('fs')

app.listen(8088);

function handler (req, res) {
  fs.readFile(__dirname + '/index.html',
  function (err, data) {
    if (err) {
      res.writeHead(500);
      return res.end('Error loading index.html');
    }

    res.writeHead(200);
    res.end(data);
  });
}

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
var reflecta = new Reflecta("COM10", function(err) {
  if (err) {
    reflecta.close(function() { done(err); });
    return;
  }

  reflecta.on('error', function(err, frame, checksum) { console.log(err + ' - ' + util.inspect(frame) + ' - ' + checksum); });
  reflecta.on('portError', function(err) { console.log(err); });
  
  reflecta.on('message', function(message) { console.log(message); });
  reflecta.on('heartbeat', function(heartbeat) {
    // Break the incoming data into floats
    var arrayBuffer = new ArrayBuffer(heartbeat.data.length);
    var byteView = new Uint8Array(arrayBuffer);
    var floatView = new Float32Array(arrayBuffer);
    
    byteView.set(heartbeat.data);
    
    var hbData = {
      gyroscope: {
        x: floatView[8],
        y: floatView[7],
        z: floatView[6]
      },
      accelerometer: {
        x: floatView[5],
        y: floatView[4],
        z: floatView[3]
      },
      magnometer: {
        x: floatView[2],
        y: floatView[1],
        z: floatView[0]
      }
    };
        
    console.log(heartbeat.collectingLoops + " : " + heartbeat.idleLoops + " : accel " + util.inspect(hbData.accelerometer) + ' : gyro ' + util.inspect(hbData.gyroscope) + ' : magno ' + util.inspect(hbData.magnometer));

    if (connected) {
      connected.emit('magnometer', { heading: hbData.magnometer.x % 360 });
    }
  });
});
