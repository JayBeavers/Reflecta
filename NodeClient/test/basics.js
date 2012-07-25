var assert = require('chai').assert;

suite('Hello', function() {
    test('Reflecta', function(done) {
        
        var Reflecta = require('../reflecta.js');
        var reflecta = new Reflecta();
        
        reflecta.on('error', function(err) { console.log(err); });
        reflecta.on('message', function(message) { console.log(message); });
        reflecta.on('response', function(response) { console.log(response.sequence + " : " + response.message); done(); });
        reflecta.on('frame', function(frame) { console.log(frame.sequence + " : " + frame.data); });
        reflecta.on('portClose', function() { console.log('portClose'); });
        reflecta.on('portEnd', function() { console.log('portEnd'); });
        reflecta.on('portError', function(err) { console.log('portError: ' + err); });

        // Send a frame once the port is open and ready to be used
        reflecta.on('portOpen', function() { reflecta.sendFrame([1]); });
        
        // Note: should consider a more 'jQuery-esque' form of this such as 'onReady' or perhaps a callback in the ctor?
    });
});
