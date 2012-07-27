var assert = require('chai').assert;

suite('Hello', function() {
    
    test('Reflecta', function(done) {
        
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
    
    test('Blinky', function(done) {

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
});
