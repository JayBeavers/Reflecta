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
      var buffer = new Buffer(7);
      buffer[0] = reflecta.FunctionIds.pushArray;
      buffer[1] = 4;
      buffer[6] = interfaceStart + 2;
      buffer.writeInt16BE(power0, 2);
      buffer.writeInt16BE(power1, 4);

      reflecta.sendFrame(buffer);
    },

    readCurrent : function(callback) {
      reflecta.sendFrame(interfaceStart + 3);
      reflecta.sendResponseCount(4, function(buffer) { 
        var currentMotor0 = buffer.readInt16BE(0);
        var currentMotor1 = buffer.readInt16BE(2);
        callback(currentMotor0, currentMotor1);
      });
    }
  };
};