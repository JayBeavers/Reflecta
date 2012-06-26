/*
  ReflectaFunctions.h - Library for binding functions to a virtual function table
*/

#include <ReflectaFramesSerial.h>

#ifndef REFLECTA_FUNCTIONS_H
#define REFLECTA_FUNCTIONS_H

// Types of errors detected by Reflecta Functions
#define FUNCTIONS_ERROR_FRAME_TOO_SMALL     0x05
#define FUNCTIONS_ERROR_FUNCTION_CONFLICT   0x06
#define FUNCTIONS_ERROR_FUNCTION_NOT_FOUND  0x07
#define FUNCTIONS_ERROR_PARAMETER_MISMATCH  0x08

// Frame Ids used by Reflecta Functions.  These are reserved values for the first byte of the frame data.
#define FUNCTIONS_QUERYINTERFACE            0x00
#define FUNCTIONS_RESPONSE                  0xFD

namespace reflectaFunctions
{
  // Bind a function to an interfaceId so it can be called by Reflecta Functions.  The assigned frame id is returned.
  // To call the function, send a frame consisting of the payload bytes:
  //   byte Function Id
  //   byte Parameter Length
  //   byte* Parameter
  byte bind(String interfaceId, void (*function)(byte callerSequence, byte parameterLength, byte* parameters));
  
  // Send a response to a function call
  //   callerSequence == the sequence number of the frame used to call the function
  //     used to correlate request/response on the caller side
  //   parameterLength & parameter byte* of the response data
  void sendResponse(byte callerSequence, byte parameterLength, byte* parameters);
  
  // reflectaFunctions setup() to be called in the Arduino setup() method
  void setup();
};

#endif
