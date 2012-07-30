/*
ReflectaHeartbeat.cpp - Library for sending timed heartbeats of data over Reflecta to a host
*/

#include "ReflectaHeartbeat.h"

namespace reflectaHeartbeat
{
  const byte heartbeatStackMax = 128;
  int heartbeatStackTop = -1;
  int8_t heartbeatStack[heartbeatStackMax + 1];

  void push(int8_t b)
  {
    if (heartbeatStackTop == heartbeatStackMax)
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_STACK_OVERFLOW);
    }
    else
    {
      heartbeatStack[++heartbeatStackTop] = b;
    }
  }

  void push16(int16_t w)
  {
    if (heartbeatStackTop > heartbeatStackMax - 2)
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_STACK_OVERFLOW);
    }
    else
    {
      heartbeatStack[++heartbeatStackTop] = (w >> 8);
      heartbeatStack[++heartbeatStackTop] = (w & 0xFF);
    }
  }
  
  byte bind(String interfaceId, void (*function)()) { return NULL; };
  
  void setFrameRate(int framesPerSecond) { };
  
  void loop() { };
  
};
