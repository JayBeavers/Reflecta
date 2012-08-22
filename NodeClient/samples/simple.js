var port = 'COM4';
var ledPin = 11;

var util = require('util');
var Reflecta = require('../reflecta.js');

var reflecta = new Reflecta(port, function(err) {

  if (reflecta.ARDU1) {
    reflecta.ARDU1.gpio.digitalWrite(ledPin, 1);
  }

});

reflecta.on('error', function(error) { console.log("e: " + error); });
reflecta.on('warning', function(warning) { console.log("w: " + warning); });
reflecta.on('message', function(message) { console.log("m: " + message); });

reflecta.on('portOpen', function() { console.log("portOpen"); });
reflecta.on('portClose', function() { console.log("portClose"); });
reflecta.on('portEnd', function() { console.log("portEnd"); });
reflecta.on('portError', function(error) { console.log("portError: " + util.inspect(error)); });