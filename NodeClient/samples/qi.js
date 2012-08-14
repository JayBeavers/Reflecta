var port = 'COM4';

var Reflecta = require('../reflecta.js');

var reflecta = new Reflecta(port, function(err) {
  console.log(reflecta.interfaces);
  process.exit();
});