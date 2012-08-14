var port = 'COM7';

var Reflecta = require('../reflecta.js');

var buffer = new Buffer(5);

var reflecta = new Reflecta(port, function(err) {

  reflecta.on('error', function(error) { console.log(error); });
  reflecta.on('message', function(message) { console.log("m: " + message); });

  buffer[0] = 0;
  buffer[1] = 2;
  buffer[4] = 5;

  buffer.writeInt16BE(500, 2);
  reflecta.sendFrame(buffer);
  reflecta.sendResponseCount(2, function(err, response) { console.log(response); console.log(response.readInt16BE(0));

  buffer.writeInt16BE(-500, 2);
  reflecta.sendFrame(buffer);
  reflecta.sendResponseCount(2, function(err, response) { console.log(response); console.log(response.readInt16BE(0));

  buffer.writeInt16BE(15, 2);
  reflecta.sendFrame(buffer);
  reflecta.sendResponseCount(2, function(err, response) { console.log(response); console.log(response.readInt16BE(0));

  buffer.writeInt16BE(-15, 2);
  reflecta.sendFrame(buffer);
  reflecta.sendResponseCount(2, function(err, response) { console.log(response); console.log(response.readInt16BE(0));

  buffer.writeInt16BE(0, 2);
  reflecta.sendFrame(buffer);
  reflecta.sendResponseCount(2, function(err, response) { console.log(response); console.log(response.readInt16BE(0));

        });
      });
    });
  });
 });
});