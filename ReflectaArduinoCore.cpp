/*
  ReflectaArduinoCore.cpp - Library for exposing the core Arduino library functions over Reflecta
*/

#include "Reflecta.h"

namespace reflectaArduinoCore {
  void pinMode() {
    int8_t pin = reflectaFunctions::pop();
    int8_t val = reflectaFunctions::pop();
    ::pinMode(pin, val);
  }

  void digitalRead() {
    int8_t pin = reflectaFunctions::pop();
    reflectaFunctions::push(::digitalRead(pin));
  }

  void digitalWrite() {
    int8_t pin = reflectaFunctions::pop();
    int8_t val = reflectaFunctions::pop();
    ::digitalWrite(pin, val);
  }

  void analogRead() {
    int8_t pin = reflectaFunctions::pop();
    reflectaFunctions::push16(::analogRead(pin));
  }

  void analogWrite() {
    int8_t pin = reflectaFunctions::pop();
    int8_t val = reflectaFunctions::pop();
    ::analogWrite(pin, val);
  }

  // TODO(jay): Support variant pulseIn(pin, value, timeout)
  void pulseIn() {
    // BUGBUG: Broken, returns a 32 bit result
    int8_t pin = reflectaFunctions::pop();
    int8_t val = reflectaFunctions::pop();
    reflectaFunctions::push(::pulseIn(pin, val));
  }

  // Bind the Arduino core methods to the ardu1 interface
  void setup() {
    reflectaFunctions::bind("ardu1", pinMode);
    reflectaFunctions::bind("ardu1", digitalRead);
    reflectaFunctions::bind("ardu1", digitalWrite);
    reflectaFunctions::bind("ardu1", analogRead);
    reflectaFunctions::bind("ardu1", analogWrite);

    reflectaFunctions::bind("ardu1", pulseIn);
  }
};  // namespace reflectaArduinoCore
