var assert = require('chai').assert;

suite('Hello', function() {
    test('Reflecta', function(done) {
        
        var Reflecta = require('../reflecta.js');
        var reflecta = new Reflecta("COM10", function(err) {
            if (err) {
              console.log(err);
              return;
            }
            
            reflecta.sendFrame([1]);
        });
        
        reflecta.on('error', function(err) { console.log(err); });
        reflecta.on('message', function(message) { console.log(message); });
        reflecta.on('response', function(response) { console.log(response.sequence + " : " + response.message); done(); });
        reflecta.on('frame', function(frame) { console.log(frame.sequence + " : " + frame.data); });
        reflecta.on('portClose', function() { console.log('portClose'); });
        reflecta.on('portEnd', function() { console.log('portEnd'); });
        reflecta.on('portError', function(err) { console.log('portError: ' + err); });
        reflecta.on('portOpen', function() { console.log('portOpen'); });
    });
});
