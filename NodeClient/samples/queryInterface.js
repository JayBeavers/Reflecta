var reflecta = require('../reflecta.js');

reflecta.detect(function(error, boards, ports) {

	if (error) {
		console.log(error);
		return;
	}

  console.log(boards[0].interfaces);
  process.exit();

})