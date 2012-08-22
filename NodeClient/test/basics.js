/*global describe */
/*global it */

var devicePath = "/dev/ttyACM0";

var assert = require('chai').assert;
var util = require('util');

describe('Hello', function() {
    
  it('Reflecta', function(done) {
      
    var Reflecta = require('../reflecta.js');
    var reflecta = new Reflecta(devicePath, function(err) {
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
      var reflecta = new Reflecta(devicePath, function(err) {
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
    var reflecta = new Reflecta(devicePath, function(err) {
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
    var reflecta = new Reflecta(devicePath, function(err) {
      if (err) {
        reflecta.close(function() { done(err); });
        return;
      }

      reflecta.on('error', function(err, frame, checksum) { console.log(err + ' - ' + util.inspect(frame) + ' - ' + checksum); });
      reflecta.on('portError', function(err) { console.log(err); });
      
      reflecta.on('message', function(message) { console.log(message); });
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
            
        console.log(heartbeat.collectingLoops + " : " + heartbeat.idleLoops + " : " + util.inspect(hbData));

      });
      
      setTimeout(function() { reflecta.close(done); }, 1700);
    });
  });

  it('RoundTripData', function(done) {
    var Reflecta = require('../reflecta.js');
    var reflecta = new Reflecta(devicePath, function(err) {
      if (err) {
        reflecta.close(function() { done(err); });
        return;
      }

      reflecta.on('error', function(err, frame, checksum) { console.log(err + ' - ' + util.inspect(frame) + ' - ' + checksum); });        
      reflecta.on('message', function(message) { console.log(message); });

      var w0 = 250;
      var w1 = -97;
      var w2 = 650;
      var w3 = -129;

      var buffer = new Buffer(10);
      buffer[0] = reflecta.FunctionIds.pushArray;
      buffer[1] = 8;
      buffer.writeInt16BE(w0, 2);
      buffer.writeInt16BE(w1, 4);
      buffer.writeInt16BE(w2, 6);
      buffer.writeInt16BE(w3, 8);

      reflecta.sendFrame(buffer);

      reflecta.sendResponseCount(8, function(error, buffer) {
        if (error) {
          reflecta.close(function() { done(error); });
          return;
        }

        var w00 = buffer.readInt16BE(0);
        var w01 = buffer.readInt16BE(2);
        var w02 = buffer.readInt16BE(4);
        var w03 = buffer.readInt16BE(6);

        assert.equal(w0, w00);
        assert.equal(w1, w01);
        assert.equal(w2, w02);
        assert.equal(w3, w03);

        done();
      });
    });
  });

  it('SimplePushResponse', function(done) {
    var Reflecta = require('../reflecta.js');
    var reflecta = new Reflecta(devicePath, function(err) {
      if (err) {
        reflecta.close(function() { done(err); });
        return;
      }

      reflecta.on('error', function(err, frame, checksum) { console.log(err); });        
      reflecta.on('message', function(message) { console.log(message); });

      reflecta.sendFrame([reflecta.FunctionIds.pushArray, 1, 99]);

      reflecta.sendResponse(function(error, buffer) {
        assert.equal(buffer[0], 99);
        assert.equal(buffer.length, 1);

        reflecta.sendFrame([reflecta.FunctionIds.pushArray, 1, 98]);

        reflecta.sendResponseCount(1, function(error, buffer) {
          assert.equal(buffer[0], 98);
          assert.equal(buffer.length, 1);

          reflecta.sendFrame([reflecta.FunctionIds.pushArray, 2, 98, 99]);

          reflecta.sendResponseCount(2, function(error, buffer) {
            assert.equal(buffer[0], 98);
            assert.equal(buffer[1], 99);
            assert.equal(buffer.length, 2);
            done();
          });

        });
      });
    });
  });

});