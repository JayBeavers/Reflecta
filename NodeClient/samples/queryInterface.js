var port = 'COM4';

var Reflecta = require('../reflecta.js');

var reflecta = new Reflecta(port);

reflecta.on('ready', function() {
  console.log(reflecta.interfaces);
  process.exit();
});