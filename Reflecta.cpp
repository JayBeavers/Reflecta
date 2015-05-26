#include "Reflecta.h"

namespace reflecta {
  void setup(int speed) {
    reflectaFrames::setup(speed);
    reflectaFunctions::setup();
    reflectaArduinoCore::setup();
  }

  void loop() {
    reflectaFrames::loop();
  }
}  // namespace reflecta
