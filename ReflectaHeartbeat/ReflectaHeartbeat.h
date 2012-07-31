/*
  ReflectaHeartbeat.h - Library for sending timed heartbeats of data over Reflecta to a host
*/

#include <Arduino.h>
#include <ReflectaFramesSerial.h>
#include <ReflectaFunctions.h>

#ifndef REFLECTA_HEARTBEAT_H
#define REFLECTA_HEARTBEAT_H

#define HEARTBEAT_MESSAGE 0x7A

namespace reflectaHeartbeat
{
  // How HB works:
  //  Has its own stack for gathering HB data
  //  Waits for fps timeout
  //  Calls array of functions asynchronously
  //    Each function returns true if done (remove, don't call again) or false for 'still running'
  //  When function array is empty, sends contents of hb stack
  //  Counts number of 'idle loops' when hb has been sent and fps timer hasn't expired
  //  Pushes 'idle loops' to the top of the stack
  
  // Gathers the HB data and sends it out when ready
  void loop();
  
  extern byte* frameTop;
  
  void push(int8_t b);
  
  void push16(int16_t w);
  
  void pushf(float f);
  
  // Bind a function to an interfaceId so it can be called by Reflecta Functions.  The assigned frame id is returned.
  void bind(bool (*function)());
  
  void setFrameRate(int framesPerSecond);
  
};

#endif
