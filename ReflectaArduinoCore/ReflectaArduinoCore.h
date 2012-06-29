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
  void pinMode(byte sequence);
  void digitalRead(byte sequence);
  void digitalWrite(byte sequence);
  void analogRead(byte sequence);
  void analogWrite(byte sequence);
  
  // Bind the Arduino core methods to the ARDU1 interface
  void setup();
};

#endif
