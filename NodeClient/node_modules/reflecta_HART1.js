// HART1 interface for the Reflecta Heartbeat

module.exports = function(reflecta, interfaceStart) {
  return {

    setFrameRate : function(rate) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 2, rate > 8, rate & 0xFF, interfaceStart] );
    }
  };
};