var util = require('util');
var Reflecta = require('./reflecta.js');
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
        
    console.log(heartbeat.freeCycles + " : accel " + util.inspect(hbData.accelerometer) + ' : gyro ' + util.inspect(hbData.gyroscope) + ' : magno ' + util.inspect(hbData.magnometer));

  });
});
