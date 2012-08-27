module.exports = function(reflecta, interfaceStart) {

  return {

    INPUT         : 0x0,
    OUTPUT        : 0x1,
    INPUT_PULLUP  : 0x2,

    HIGH          : 0x1,
    LOW           : 0x0,

    pinMode : function(pin, mode) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 2, pin, mode, interfaceStart] );
    },

    digitalRead : function(pin, callback) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, pin, interfaceStart + 1] );
      reflecta.sendResponse(function(buffer) {
        callback(buffer[0]);
      });
    },

    digitalWrite : function(pin, value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 2, pin, value, interfaceStart + 2] );
    },

    analogRead : function(pin, callback) {
      reflecta.sendFrame(reflecta.FunctionIds.pushArray, 1, pin, interfaceStart + 3);
      reflecta.sendResponseCount(2, function(buffer) {
        var analogValue = buffer.readInt16BE(0);
        callback(analogValue);
      });
    },

    analogWrite : function(pin, value) {
      reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 2, pin, value, interfaceStart + 4] );
    },

    Wire : {

      begin : function() {
        reflecta.sendFrame(interfaceStart + 5);
      },

      requestFrom : function(address, quantity) {
        reflecta.sendFrame(reflecta.FunctionIds.pushArray, 2, address, quantity, interfaceStart + 6);
      },

      requestFromStart : function(address, quantity) {
        reflecta.sendFrame(reflecta.FunctionIds.pushArray, 2, address, quantity, interfaceStart + 7);
      },

      available : function(callback) {
        reflecta.sendFrame(interfaceStart + 8);
        reflecta.sendResponse(function(buffer) {
          callback(buffer[0]);
        });
      },

      read : function() {
        reflecta.sendFrame(interfaceStart + 9);
        reflecta.sendResponse(function(buffer) {
          callback(buffer[0]);
        });
      },

      beginTransmission : function(address) {
        reflecta.sendFrame(reflecta.FunctionIds.pushArray, 1, address, interfaceStart + 10);
      },

      // TODO: Support variants write(string) and write(data, length)
      write : function(value) {
        reflecta.sendFrame(reflecta.FunctionIds.pushArray, 1, value, interfaceStart + 11);
      },

      endTransmission : function() {
        reflecta.sendFrame(interfaceStart + 12);
      }
    },

    Servo : {
      // TODO: Support variant attach(pin, min, max)
      attach : function(pin) {
        reflecta.sendFrame(reflecta.FunctionIds.pushArray, 1, pin, interfaceStart + 13);
      },

      detach : function(pin) {
        reflecta.sendFrame(reflecta.FunctionIds.pushArray, 1, pin, interfaceStart + 14);
      },

      write : function(pin, angle) {
        reflecta.sendFrame(reflecta.FunctionIds.pushArray, 2, pin, angle, interfaceStart + 15);
      },

      writeMicroseconds : function(pin, uS) {

        var buffer = new Buffer(5);
        buffer[0] = reflecta.FunctionIds.pushArray;
        buffer[1] = pin;
        buffer.writeInt16BE(uS, 2);
        buffer[4] = interfaceStart + 16;

        reflecta.sendFrame(buffer);
      }
    }
  };
};