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
  
  void pushf(float f)
  {
    if (heartbeatStackTop > heartbeatStackMax - 4)
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_STACK_OVERFLOW);
    }
    else
    {
      byte* pb = (byte*)&f;
      heartbeatStack[++heartbeatStackTop] = pb[0];
      heartbeatStack[++heartbeatStackTop] = pb[1];
      heartbeatStack[++heartbeatStackTop] = pb[2];
      heartbeatStack[++heartbeatStackTop] = pb[3];
    }
  }
  
  int8_t pop()
  {
    if (heartbeatStackTop == -1)
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_STACK_UNDERFLOW);
      return -1;
    }
    else
    {
      return heartbeatStack[heartbeatStackTop--];
    }
  }

  
  const int MaxFunctions = 16;
  
  // Function table of all functions to call in heartbeat
  bool (*heartbeatFunctions[MaxFunctions])();

  // Has the function finished for this heartbeat
  bool functionComplete[MaxFunctions];

  int functionsTop = 0;
  void bind(bool (*function)()) {
    heartbeatFunctions[functionsTop++] = function;
  };
  
  uint16_t millisBetweenFrames = 10;
  
  void setFrameRate(int framesPerSecond) { millisBetweenFrames = 1000 / framesPerSecond; };
  
  uint16_t idleLoops = 0;
  void sendHeartbeat() {
      push16(idleLoops);
      push(HEARTBEAT_MESSAGE);
      
      byte frame[heartbeatStackTop + 1];
      
      for (int i = 0; i < heartbeatStackTop + 1; i++)
      {
        frame[i] = pop();
      }
      
      reflectaFrames::sendFrame(frame, heartbeatStackTop + 1);
  };
  
  bool finished = false;
  unsigned long nextFrame = 0;
  void loop() {
    
    if (!finished) {
      finished = true;
      for (int i = 0; i < functionsTop; i++) {
        if (!functionComplete[i]) {
          functionComplete[i] = heartbeatFunctions[i]();
          if (!functionComplete[i]) finished = false;
        }
      }
    }
    
    if (finished) {
      unsigned long currentTime = millis();
      if (currentTime >= nextFrame) {
        sendHeartbeat();
        
        nextFrame = currentTime + millisBetweenFrames;
        idleLoops = 0;
        
        for (int i = 0; i < MaxFunctions; i++) functionComplete[i] = false;
        finished = false;
        
      } else {
        idleLoops++;
      }
    }
    
  };
};
