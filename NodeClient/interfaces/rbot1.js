// RBOT1 interface for the RocketBot Rocket Base Arduino project
// Implements node.js API for the RocketBot's controller.

module.exports = function(reflecta, interfaceStart) {
  return {

    AnimationCommand : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart] );
    },

    FireCommand : function() {
      reflecta.sendFrame( [value,interfaceStart + 1] );
    },

    LightCommand : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 2] );
    },

    PanCommand : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 3] );
    },

    TiltCommand : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 4] );
    },

    CompressorCommand : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 5] );
    },

    ValveCommand : function(value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, value,interfaceStart + 6] );
    }
  }
}