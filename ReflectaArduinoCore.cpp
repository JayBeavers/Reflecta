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

    reflectaFunctions::bind("ardu1", wireBeginMaster);

    reflectaFunctions::bind("ardu1", wireRequestFrom);
    reflectaFunctions::bind("ardu1", wireRequestFromStart);
    reflectaFunctions::bind("ardu1", wireAvailable);
    reflectaFunctions::bind("ardu1", wireRead);

    reflectaFunctions::bind("ardu1", wireBeginTransmission);
    reflectaFunctions::bind("ardu1", wireWrite);
    reflectaFunctions::bind("ardu1", wireEndTransmission);

    reflectaFunctions::bind("ardu1", servoAttach);
    reflectaFunctions::bind("ardu1", servoDetach);
    reflectaFunctions::bind("ardu1", servoWrite);
    reflectaFunctions::bind("ardu1", servoWriteMicroseconds);

    reflectaFunctions::bind("ardu1", pulseIn);
  }
};  // namespace reflectaArduinoCore
