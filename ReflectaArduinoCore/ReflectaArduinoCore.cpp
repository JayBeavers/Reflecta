/*
  ReflectaArduinoCore.cpp - Library for exposing the core Arduino library functions over Reflecta
*/

#include <Arduino.h>
#include <ReflectaFramesSerial.h>
#include <ReflectaFunctions.h>
#include "ReflectaArduinoCore.h"

using namespace reflectaFunctions;

namespace reflectaArduinoCore
{
  void pinMode(byte sequence)
  {
    ::pinMode(pop(), pop());
  }
  
  void digitalRead(byte sequence)
  {
    byte val = ::digitalRead(pop());
    sendResponse(sequence, 1, &val);
  }
  
  void digitalWrite(byte sequence)
  {
    ::digitalWrite(pop(), pop());
  }
  
  void analogRead(byte sequence)
  {
    int16_t val = ::analogRead(pop());
    sendResponse(sequence, 2, (byte*)&val);
  }
  
  void analogWrite(byte sequence)
  {
    ::analogWrite(pop(), pop());
  }
  
  // Bind the Arduino core methods to the ARDU1 interface
  void setup()
  {
    reflectaFunctions::bind("ARDU1", pinMode);
    reflectaFunctions::bind("ARDU1", digitalRead);
    reflectaFunctions::bind("ARDU1", digitalWrite);
    reflectaFunctions::bind("ARDU1", analogRead);
    reflectaFunctions::bind("ARDU1", analogWrite);
  }
};
