var assert = require('chai').assert;
var util = require('util');

describe('Hello', function() {
    
    it('Reflecta', function(done) {
        
      var Reflecta = require('../reflecta.js');
      var reflecta = new Reflecta("COM10", function(err) {
        if (err) {
          reflecta.close(function() { done(err); });
          return;
        }

        reflecta.on('response', function(response) {
          // console.log(response.sequence + " : " + response.message);
          reflecta.close(done);
        });
        
        reflecta.sendFrame([1]);
      });
      
      reflecta.on('error', function(err) { console.log(err); });
      reflecta.on('portError', function(err) { console.log(err); });

      reflecta.on('message', function(message) { console.log(message); });
      reflecta.on('frame', function(frame) { console.log(frame.sequence + " : " + frame.data); });
      reflecta.on('portClose', function() { console.log('portClose'); });
      reflecta.on('portEnd', function() { console.log('portEnd'); });
      reflecta.on('portOpen', function() { console.log('portOpen'); });
    });
    
    it('Blinky', function(done) {

        var Reflecta = require('../reflecta.js');
        var reflecta = new Reflecta("COM10", function(err) {
          if (err) {
            reflecta.close(function() { done(err); });
            return;
          }

          reflecta.on('error', function(err) { console.log(err); });
          reflecta.on('portError', function(err) { console.log(err); });
          
          setInterval(function() { reflecta.ardu1.gpio.digitalWrite(11, 1); }, 199);
          setInterval(function() { reflecta.ardu1.gpio.digitalWrite(11, 0); }, 400);
          
          setTimeout(function() { reflecta.close(done); }, 1700);
        });
    });
    
    // Meant to be run when Reflecta has the ReflectaHeartbeat turned on and running
    it('BasicHeartbeat', function(done) {

      var Reflecta = require('../reflecta.js');
      var reflecta = new Reflecta("COM10", function(err) {
        if (err) {
          reflecta.close(function() { done(err); });
          return;
        }

        reflecta.on('error', function(err, frame, checksum) { console.log(err + ' - ' + util.inspect(frame) + ' - ' + checksum); });
        reflecta.on('portError', function(err) { console.log(err); });
        
        reflecta.on('message', function(message) { console.log(message); });
        reflecta.on('frame', function(frame) { console.log(frame.sequence + " : " + frame.data + ' : ' + util.inspect(frame.frame)); });
        
        setTimeout(function() { reflecta.close(done); }, 1700);
      });
    });

    // Meant to be run as the client for BasicIMU
    it('HeartbeatIMU', function(done) {

      var Reflecta = require('../reflecta.js');
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
              
          console.log(heartbeat.freeCycles + " : " + util.inspect(hbData));

        });
        
        setTimeout(function() { reflecta.close(done); }, 1700);
      });
    });
});
