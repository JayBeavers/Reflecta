#pragma once

#include <Servo.h>
#include "Reflecta.h"

namespace reflectaServo
{
  // basic methods
  void setup();

  // interface methods
  void servoAttach();
  void servoDetach();
  void servoWrite();
  void servoWriteMicroseconds();
}; // namespace reflectaServo