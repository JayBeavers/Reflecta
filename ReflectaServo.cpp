#include "Reflecta.h"
#include "ReflectaServo.h"

namespace ReflectaServo {
  Servo servos[MAX_SERVOS];

  // TODO(jay): Support variant attach(pin, min, max)
  void servoAttach() {
    int8_t pin = reflectaFunctions::pop();
    servos[pin].attach(pin);
  }

  void servoDetach() {
    int8_t pin = reflectaFunctions::pop();
    servos[pin].detach();
  }

  void servoWrite() {
    int8_t pin = reflectaFunctions::pop();
    int8_t val = reflectaFunctions::pop();
    servos[pin].write(val);
  }

  void servoWriteMicroseconds() {
    int8_t pin = reflectaFunctions::pop();
    int8_t val = reflectaFunctions::pop16();
    servos[pin].writeMicroseconds(val);
  }

  void setup() {
    reflectaFunctions::bind("srvo1", servoAttach);
    reflectaFunctions::bind("srvo1", servoDetach);
    reflectaFunctions::bind("srvo1", servoWrite);
    reflectaFunctions::bind("srvo1", servoWriteMicroseconds);
  }
};  // namespace reflectaServo
