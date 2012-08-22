var util = require('util');
var Reflecta = require('./reflecta.js');
var reflecta = new Reflecta("COM10");

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
  });
});

reflecta.on('error', function(err, frame, checksum) { console.log(err + ' - ' + util.inspect(frame) + ' - ' + checksum); });
reflecta.on('portError', function(err) { console.log(err); });

reflecta.on('message', function(message) { console.log(message); });