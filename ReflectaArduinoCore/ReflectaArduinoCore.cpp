/*
  ReflectaArduinoCore.cpp - Library for exposing the core Arduino library functions over Reflecta
*/

#include <Arduino.h>
#include <ReflectaFramesSerial.h>
#include <ReflectaFunctions.h>
#include "ReflectaArduinoCore.h"

namespace reflectaArduinoCore
{
  void pinMode(byte sequence, byte parameterLength, byte* parameters)
  {
  	if (parameterLength != 2)
  	{
  	  reflectaFrames::sendError(FUNCTIONS_ERROR_PARAMETER_MISMATCH);
  	}
  	
    ::pinMode(parameters[0], parameters[1]);
  }
  
  void digitalRead(byte sequence, byte parameterLength, byte* parameters)
  {
  	if (parameterLength != 1)
  	{
  		reflectaFrames::sendError(FUNCTIONS_ERROR_PARAMETER_MISMATCH);
  	}

    byte val = ::digitalRead(parameters[0]);
    reflectaFunctions::sendResponse(sequence, 1, &val);
  }
  
  void digitalWrite(byte sequence, byte parameterLength, byte* parameters)
  {
  	if (parameterLength != 2)
  	{
  		reflectaFrames::sendError(FUNCTIONS_ERROR_PARAMETER_MISMATCH);
  	}

    ::digitalWrite(parameters[0], parameters[1]);
  }
  
  void analogRead(byte sequence, byte parameterLength, byte* parameters)
  {
  	if (parameterLength != 1)
  	{
  	  reflectaFrames::sendError(FUNCTIONS_ERROR_PARAMETER_MISMATCH);
  	}

    int16_t val = ::analogRead(parameters[0]);
    reflectaFunctions::sendResponse(sequence, 2, (byte*)&val);
  }
  
  void analogWrite(byte sequence, byte parameterLength, byte* parameters)
  {
  	if (parameterLength != 2)
  	{
  		reflectaFrames::sendError(FUNCTIONS_ERROR_PARAMETER_MISMATCH);
  	}

    ::analogWrite(parameters[0], parameters[1]);
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
