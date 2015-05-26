/*
  ReflectaArduinoCore.cpp - Library for exposing the core Arduino library functions over Reflecta
*/

#include "Reflecta.h"

using namespace reflecta;
using namespace reflectaFunctions;

namespace reflectaArduinoCore
{
  void pinMode()
  {
    int8_t pin = pop();
    int8_t val = pop();
    ::pinMode(pin, val);
  }

  void digitalRead()
  {
    int8_t pin = pop();
    push(::digitalRead(pin));
  }

  void digitalWrite()
  {
    int8_t pin = pop();
    int8_t val = pop();
    ::digitalWrite(pin, val);
  }

  void analogRead()
  {
    int8_t pin = pop();
    push16(::analogRead(pin));
  }

  void analogWrite()
  {
    int8_t pin = pop();
    int8_t val = pop();
    ::analogWrite(pin, val);
  }

  void wireBeginMaster()
  {
    Wire.begin();
  }

  void wireRequestFrom()
  {
    int8_t address = pop();
    int8_t quantity = pop();
    Wire.requestFrom(address, quantity);
  }

  void wireRequestFromStart()
  {
    int8_t address = pop();
    int8_t quantity = pop();
    Wire.requestFrom(address, quantity, false);
  }

  void wireAvailable()
  {
    push(Wire.available());
  }

  void wireRead()
  {
    if (Wire.available())
      push(Wire.read());
    else
      reflectaFrames::sendEvent(Error, WireNotAvailable);
  }

  void wireBeginTransmission()
  {
    int8_t address = pop();
        Wire.beginTransmission(address);
  }

  // TODO: Support variants write(string) and write(data, length)
  void wireWrite()
  {
    int8_t val = pop();
    Wire.write(val);
  }

  void wireEndTransmission()
  {
    Wire.endTransmission();
  }

  Servo servos[MAX_SERVOS];

  // TODO: Support variant attach(pin, min, max)
  void servoAttach()
  {
      int8_t pin = pop();
      servos[pin].attach(pin);
  }

  void servoDetach()
  {
    int8_t pin = pop();
      servos[pin].detach();
  }

  void servoWrite()
  {
    int8_t pin = pop();
    int8_t val = pop();
      servos[pin].write(val);
  }

  void servoWriteMicroseconds()
  {
    int8_t pin = pop();
    int8_t val = pop16();
      servos[pin].writeMicroseconds(val);
  }

  // TODO: Support variant pulseIn(pin, value, timeout)
  void pulseIn()
  {
    // BUGBUG: Broken, returns a 32 bit result
    int8_t pin = pop();
    int8_t val = pop();
    push(::pulseIn(pin, val));
  }

  // Bind the Arduino core methods to the ardu1 interface
  void setup()
  {
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
};
