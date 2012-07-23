/*
  ReflectaArduinoCore.cpp - Library for exposing the core Arduino library functions over Reflecta
*/

#include "ReflectaArduinoCore.h"

#define MAX_SERVOS        48   // max number of servos

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
      reflectaFrames::sendError(ARDUINO_CORE_ERROR_WIRE_NOT_AVAILABLE);
  }
  
  void wireBeginTransmission()
  {
    Wire.beginTransmission(pop());
  }
  
  void wireWrite()
  {
    Wire.write(pop());
  }
  
  void wireEndTransmission()
  {
    Wire.endTransmission();
  }
  
  Servo servos[MAX_SERVOS];

  void servoAttach()
  {
      int16_t pin = pop();
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
  
  void pulseIn()
  {
    push(::pulseIn(pop(), pop()));
  }
  
  // Bind the Arduino core methods to the ARDU1 interface
  void setup()
  {
    reflectaFunctions::bind("ARDU1", pinMode);
    reflectaFunctions::bind("ARDU1", digitalRead);
    reflectaFunctions::bind("ARDU1", digitalWrite);
    reflectaFunctions::bind("ARDU1", analogRead);
    reflectaFunctions::bind("ARDU1", analogWrite);

    reflectaFunctions::bind("ARDU1", wireBeginMaster);
    
    reflectaFunctions::bind("ARDU1", wireRequestFrom);
    reflectaFunctions::bind("ARDU1", wireRequestFromStart);
    reflectaFunctions::bind("ARDU1", wireAvailable);
    reflectaFunctions::bind("ARDU1", wireRead);
    
    reflectaFunctions::bind("ARDU1", wireBeginTransmission);
    reflectaFunctions::bind("ARDU1", wireWrite);
    reflectaFunctions::bind("ARDU1", wireEndTransmission);
    
    reflectaFunctions::bind("ARDU1", servoAttach);
    reflectaFunctions::bind("ARDU1", servoDetach);
    reflectaFunctions::bind("ARDU1", servoWrite);
    reflectaFunctions::bind("ARDU1", servoWriteMicroseconds);

    reflectaFunctions::bind("ARDU1", pulseIn);
  }
};
