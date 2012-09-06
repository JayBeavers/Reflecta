var ledPin = 11;

var reflecta = require('../reflecta.js');

reflecta.detect(function(error, boards, ports) {

	if (error) {
		console.log(error);
		return;
	}

	var board = boards[0];

	board.on('error', function(error) { console.log("e: " + error); });
	board.on('warning', function(warning) { console.log("w: " + warning); });
	board.on('message', function(message) { console.log("m: " + message); });

	board.on('close', function() { console.log("close"); });
	board.on('open', function() { console.log("open"); });
	board.on('end', function() { console.log("end"); });

  if (board.ARDU1) {
  	board.ARDU1.pinMode(ledPin, board.ARDU1.OUTPUT);
    board.ARDU1.digitalWrite(ledPin, 1);
  }

});