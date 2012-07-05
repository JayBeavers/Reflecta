/*
  ReflectaArduinoCore.h - Library for exposing the core Arduino library functions over Reflecta
*/

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <ReflectaFramesSerial.h>
#include <ReflectaFunctions.h>

#ifndef REFLECTA_ARDUINO_CORE_H
#define REFLECTA_ARDUINO_CORE_H

#define ARDUINO_CORE_ERROR_WIRE_NOT_AVAILABLE     0x0B

namespace reflectaArduinoCore
{
  // ReflectaFunctions wrappers that receive the function call, parse the
  //   parameters, and call the matching Arduino libray functions
  void pinMode();
  void digitalRead();
  void digitalWrite();
  void analogRead();
  void analogWrite();
  
  void wireBeginMaster();
  
  void wireRequestFrom();
  void wireRequestFromStart();
  void wireAvailable();
  void wireRead();
  
  void wireBeginTransmission();
  void wireWrite();
  void wireEndTransmission();
  
  void servoAttach();
  void servoDetach();
  void servoWriteMicroseconds();
  
  void pulseIn();
  
  // Bind the Arduino core methods to the ARDU1 interface
  void setup();
};

#endif
