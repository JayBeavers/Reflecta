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
    ::pinMode(pop(), pop());
  }
  
  void digitalRead()
  {
    push(::digitalRead(pop()));
  }
  
  void digitalWrite()
  {
    ::digitalWrite(pop(), pop());
  }
  
  void analogRead()
  {
    push16(::analogRead(pop()));
  }
  
  void analogWrite()
  {
    ::analogWrite(pop(), pop());
  }
  
  void wireBeginMaster()
  {
    Wire.begin();
  }
  
  void wireRequestFrom()
  {
    Wire.requestFrom(pop(), pop());
  }
  
  void wireRequestFromStart()
  {
    Wire.requestFrom(pop(), pop(), false);
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
    Wire.beginTransmission(pop());
  }
  
  // TODO: Support variants write(string) and write(data, length)
  void wireWrite()
  {
    Wire.write(pop());
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
      servos[pop()].detach();
  }

  void servoWrite()
  {
      servos[pop()].write(pop());
  }
  
  void servoWriteMicroseconds()
  {
      servos[pop()].writeMicroseconds(pop16());
  }
  
  // TODO: Support variant pulseIn(pin, value, timeout)
  void pulseIn()
  {
    // BUGBUG: Broken, returns a 32 bit result
    push(::pulseIn(pop(), pop()));
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
