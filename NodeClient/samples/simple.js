var port = 'COM4';
var ledPin = 11;

var Reflecta = require('../reflecta.js');

var reflecta = new Reflecta(port, function(err) {

  if (reflecta.ARDU1) {
    reflecta.ARDU1.gpio.digitalWrite(ledPin, 1);
  }

});