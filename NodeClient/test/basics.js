var devicePath = "COM4";

var assert = require('chai').assert;
var util = require('util');
var Reflecta = require('../reflecta.js');

var reflectaTestFactory = function(done) {

  var reflecta = new Reflecta(devicePath);

  reflecta.on('error', function(error) {
    console.log("e: " + error);
    reflecta.close(function() {
      done(error);
    });
  });

  reflecta.on('warning', function(warning) { console.log("w: " + warning); });
  reflecta.on('message', function(message) { console.log("m: " + message); });
  reflecta.on('close', function() { console.log('close'); });
  reflecta.on('end', function() { console.log('end'); });
  reflecta.on('open', function() { console.log('open'); });

  return reflecta;
};

describe('Basic Reflexes', function() {
    
  it('QueryInterface Responds', function(done) {
      
    var reflecta = reflectaTestFactory(done);

    reflecta.on('ready', function() {

      reflecta.on('response', function(response) {
        reflecta.removeAllListeners('error');
        reflecta.close(done);
      });
      
      reflecta.sendFrame(reflecta.FunctionIds.queryInterface);

    });
    

  });
  
  it('ARDU1 Blinky works', function(done) {

    var reflecta = reflectaTestFactory(done);

    reflecta.on('ready', function() {

      var count = 0;

      var idOn = setInterval(function() { reflecta.ARDU1.digitalWrite(11, 1); count++; }, 199);
      var idOff = setInterval(function() {
        reflecta.ARDU1.digitalWrite(11, 0);
        if (count > 5) {
          clearInterval(idOn);
          clearInterval(idOff);
        }
      }, 400);
      
      setTimeout(function() {
        reflecta.removeAllListeners('error');
        reflecta.close(done);
      }, 1700);
    });

  });

  it('PushArray and SendResponseCount properly round trip', function(done) {

    var reflecta = reflectaTestFactory(done);

    reflecta.on('ready', function() {

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

      reflecta.sendResponseCount(8, function(buffer) {
        var w00 = buffer.readInt16BE(0);
        var w01 = buffer.readInt16BE(2);
        var w02 = buffer.readInt16BE(4);
        var w03 = buffer.readInt16BE(6);

        assert.equal(w0, w00);
        assert.equal(w1, w01);
        assert.equal(w2, w02);
        assert.equal(w3, w03);

        reflecta.removeAllListeners('error');
        reflecta.close(done);
      });
    });
  });

  it('Simple PushArray and SendResponse properly round trip', function(done) {

    var reflecta = reflectaTestFactory(done);

    reflecta.on('ready', function() {

      reflecta.sendFrame([reflecta.FunctionIds.pushArray, 1, 99]);

      reflecta.sendResponse(function(buffer) {
        assert.equal(buffer[0], 99);
        assert.equal(buffer.length, 1);

        reflecta.sendFrame([reflecta.FunctionIds.pushArray, 1, 98]);

        reflecta.sendResponseCount(1, function(buffer) {
          assert.equal(buffer[0], 98);
          assert.equal(buffer.length, 1);

          reflecta.sendFrame([reflecta.FunctionIds.pushArray, 2, 98, 99]);

          reflecta.sendResponseCount(2, function(buffer) {
            assert.equal(buffer[0], 98);
            assert.equal(buffer[1], 99);
            assert.equal(buffer.length, 2);
            
            reflecta.removeAllListeners('error');
            reflecta.close(done);
          });
        });
      });
    });
  });
});