Overview
========

Reflecta is a node.js client for communicating with an Arduino via the Reflecta protocol.

### Sample Usage

> var Reflecta = require('./reflecta');

> var reflecta = new Reflecta();

> reflecta.on('response', function(response) { console.log(response.sequence + ' : ' + response.message); });

> reflecta.on('portOpen', function() { reflecta.sendFrame([1]); });