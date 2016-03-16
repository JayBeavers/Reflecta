#include "Reflecta.h"
#include "ReflectaWire.h"

namespace reflectaWire {
  void wireBeginMaster() {
    Wire.begin();
  }

  void wireRequestFrom() {
    int8_t address = reflectaFunctions::pop();
    int8_t quantity = reflectaFunctions::pop();
    Wire.requestFrom(address, quantity);
  }

  void wireRequestFromStart() {
    int8_t address = reflectaFunctions::pop();
    int8_t quantity = reflectaFunctions::pop();
    Wire.requestFrom(address, quantity, false);
  }

  void wireAvailable() {
    reflectaFunctions::push(Wire.available());
  }

  void wireRead() {
    if (Wire.available())
    reflectaFunctions::push(Wire.read());
    else
      reflectaFrames::sendEvent(reflecta::Error, reflecta::WireNotAvailable);
  }

  void wireBeginTransmission() {
    int8_t address = reflectaFunctions::pop();
    Wire.beginTransmission(address);
  }

  // TODO(jay): Support variants write(string) and write(data, length)
  void wireWrite() {
    int8_t val = reflectaFunctions::pop();
    Wire.write(val);
  }

  void wireEndTransmission() {
    Wire.endTransmission();
  }

  void setup() {
    reflectaFunctions::bind("wire1", wireBeginMaster);
    reflectaFunctions::bind("wire1", wireRequestFrom);
    reflectaFunctions::bind("wire1", wireRequestFromStart);
    reflectaFunctions::bind("wire1", wireAvailable);
    reflectaFunctions::bind("wire1", wireRead);
    reflectaFunctions::bind("wire1", wireBeginTransmission);
    reflectaFunctions::bind("wire1", wireWrite);
    reflectaFunctions::bind("wire1", wireEndTransmission);
  }
};  // namespace reflectaArduinoCore
