/*
  ReflectaFunctions.h - Library for binding functions to a virtual function table
*/

#include <Arduino.h>

#ifndef REFLECTA_FUNCTIONS_H
#define REFLECTA_FUNCTIONS_H

namespace reflectaFunctions
{
  enum EventCode {
    FrameTooSmall       = 0x05,
    FunctionConflict    = 0x06,
    FunctionNotFound    = 0x07,
    ParameterMismatch   = 0x08,
    StackOverflow       = 0x09,
    StackUnderflow      = 0x0A
  };

  // Frame Ids used by Reflecta Functions.  These are reserved values for the first byte of the frame data.
  enum FunctionId {
    PushArray           = 0x00,
    QueryInterface      = 0x01,
    SendResponse        = 0x02,
    SendResponseCount   = 0x03,
    Response            = 0x7C
  };

  // Bind a function to an interfaceId so it can be called by Reflecta Functions.  The assigned frame id is returned.
  byte bind(String interfaceId, void (*function)());
  
  void push(int8_t b);
  
  void push16(int16_t b);
  
  int8_t pop();

  int16_t pop16();

  // Send a response to a function call
  //   callerSequence == the sequence number of the frame used to call the function
  //     used to correlate request/response on the caller side
  //   parameterLength & parameter byte* of the response data
  void sendResponse(byte parameterLength, byte* parameters);
  
  // reflectaFunctions setup() to be called in the Arduino setup() method
  void setup();
  
  // Execution pointer for Reflecta Functions.  To be used by functions that
  // change the order of instruction execution in the incoming frame.  Note:
  // if you are not implementing your own 'scripting language', you shouldn't
  // be using this.
  extern byte* execution;
  
  // Top of the frame marker to be used when modifying the execution pointer.
  // Generally speaking execution should not go beyong frameTop.  When
  // execution == frameTop, the Reflecta Functions frameReceived execution loop
  // stops. 
  extern byte* frameTop;
};

#endif
