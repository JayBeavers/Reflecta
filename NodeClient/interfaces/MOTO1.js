// MOTO1 interface for the RocketBot Drive Base Teensy project
// Implements node.js API for the SparkFun Monster Moto Board

module.exports = function(reflecta, interfaceStart) {
  return {

    brakeGround : function() {
      reflecta.sendFrame( [interfaceStart] );
    },

    brakeVcc : function() {
      reflecta.sendFrame( [interfaceStart + 1] );
    },

    drive : function(power0, power1) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 2, power0, power1, interfaceStart + 2] );
    },

    readCurrent : function(callback) {
      reflecta.sendFrame(interfaceStart + 3);
      reflecta.sendResponseCount(4, function(buffer) { 
        var currentMotor0 = buffer[0] < 8 + buffer[1];
        var currentMotor1 = buffer[2] < 8 + buffer[3];
        callback(currentMotor0, currentMotor1);
      });
    }
  }
}