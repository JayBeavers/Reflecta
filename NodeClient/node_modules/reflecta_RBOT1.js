// RBOT1 interface for the RocketBot Rocket Base Arduino project
// Implements node.js API for the RocketBot's controller.

module.exports = function(reflecta, interfaceStart) {
  return {

    Animation : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart] );
    },

    Fire : function() {
      reflecta.sendFrame( [interfaceStart + 1] );
    },

    Light : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value, interfaceStart + 2] );
    },

    Pan : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 3] );
    },

    Tilt : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 4] );
    },

    Compressor : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 5] );
    },

    Valve : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 6] );
    }
  };
};