var port = 'COM4';
var ledPin = 11;

var util = require('util');
var Reflecta = require('../reflecta.js');

var reflecta = new Reflecta(port);

reflecta.on('ready', function() {

  if (reflecta.ARDU1) {
    reflecta.ARDU1.digitalWrite(ledPin, 1);
  }
});

reflecta.on('error', function(error) { console.log("e: " + error); });
reflecta.on('warning', function(warning) { console.log("w: " + warning); });
reflecta.on('message', function(message) { console.log("m: " + message); });

reflecta.on('close', function() { console.log("close"); });
reflecta.on('open', function() { console.log("open"); });
reflecta.on('end', function() { console.log("end"); });