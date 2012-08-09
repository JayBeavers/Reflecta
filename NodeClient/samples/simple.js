var port = 'COM4';
var ledPin = 11;

var Reflecta = require('../reflecta.js');

var reflecta = new Reflecta(port, function(err) {

  reflecta.ardu1.gpio.digitalWrite(ledPin, 1);
  process.exit();

});