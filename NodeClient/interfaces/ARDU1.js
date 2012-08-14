module.exports = function(reflecta, interfaceStart) {

  return {

    gpio : {

      pinMode : function(pin, mode) {
        reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 2, pin, mode, interfaceStart] );
      },

      digitalRead : function(pin, callback) {
        reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 1, pin, interfaceStart + 1] );
        reflecta.sendResponse(callback);
      },

      digitalWrite : function(pin, value) {
        reflecta.sendFrame( [reflecta.FunctionIds.pushArray, 2, pin, value, interfaceStart + 2] );
      },

      analogRead : function() {
        reflecta.sendFrame(interfaceStart + 3);
      },

      analogWrite : function() {
        reflecta.sendFrame(interfaceStart + 4);
      },

      pulseIn : function() {
        reflecta.sendFrame(interfaceStart + 17);
      }
      
    },
    
    wire : {
      beginMaster : function() {
        reflecta.sendFrame(interfaceStart + 5);
      },
      requestFrom : function() {
        reflecta.sendFrame(interfaceStart + 6);
      },
      requestFromStart : function() {
        reflecta.sendFrame(interfaceStart + 7);
      },
      available : function() {
        reflecta.sendFrame(interfaceStart + 8);
      },
      read : function() {
        reflecta.sendFrame(interfaceStart + 9);
      },
      beginTransmission : function() {
        reflecta.sendFrame(interfaceStart + 10);
      },
      write : function() {
        reflecta.sendFrame(interfaceStart + 11);
      },
      endTransmission : function() {
        reflecta.sendFrame(interfaceStart + 12);
      }
    },
    
    servo : {
      attach : function() {
        reflecta.sendFrame(interfaceStart + 13);
      },
      detach : function() {
        reflecta.sendFrame(interfaceStart + 14);
      },
      write : function() {
        reflecta.sendFrame(interfaceStart + 15);
      },
      writeMicroseconds : function() {
        reflecta.sendFrame(interfaceStart + 16);
      }
    }

  }
}