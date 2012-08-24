var port = 'COM4';

var Reflecta = require('../reflecta.js');

var reflecta = new Reflecta(port);

reflecta.on('ready', function() {
  reflecta.ARDU1.digitalWrite(11, 1);
  reflecta.MOTO1.drive(50, -250);
});

reflecta.on('error', function(err) { console.log(err); });
reflecta.on('warning', function(warning) { console.log(warning); });
reflecta.on('message', function(message) { console.log(message); });