/*
  ReflectaArduinoCore.h - Library for exposing the core Arduino library functions over Reflecta
*/

#include <ReflectaFunctions.h>

#ifndef REFLECTA_ARDUINO_CORE_H
#define REFLECTA_ARDUINO_CORE_H

namespace reflectaArduinoCore
{
  // ReflectaFunctions wrappers that receive the function call, parse the
  //   parameters, and call the matching Arduino libray functions
  void pinMode(byte sequence, byte parameterLength, byte* parameters);
  void digitalRead(byte sequence, byte parameterLength, byte* parameters);
  void digitalWrite(byte sequence, byte parameterLength, byte* parameters);
  void analogRead(byte sequence, byte parameterLength, byte* parameters);
  void analogWrite(byte sequence, byte parameterLength, byte* parameters);
  
  // Bind the Arduino core methods to the ARDU1 interface
  void setup();
};

#endif
