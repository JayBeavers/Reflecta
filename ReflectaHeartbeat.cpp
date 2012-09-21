/*
ReflectaHeartbeat.cpp - Library for sending timed heartbeats of data over Reflecta to a host
*/

#include "Reflecta.h"

using namespace reflecta;

namespace reflectaHeartbeat
{
  const byte heartbeatStackMax = 128;
  int heartbeatStackTop = -1;
  int8_t heartbeatStack[heartbeatStackMax + 1];

  void push(int8_t b)
  {
    if (heartbeatStackTop == heartbeatStackMax)
    {
      reflectaFrames::sendEvent(Error, StackOverflow);
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
      reflectaFrames::sendEvent(Error, StackOverflow);
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
      reflectaFrames::sendEvent(Error, StackOverflow);
    }
    else
    {
      byte* pb = (byte*)&f;
      heartbeatStack[++heartbeatStackTop] = pb[3];
      heartbeatStack[++heartbeatStackTop] = pb[2];
      heartbeatStack[++heartbeatStackTop] = pb[1];
      heartbeatStack[++heartbeatStackTop] = pb[0];
    }
  }
  
  int8_t pop()
  {
    if (heartbeatStackTop == -1)
    {
      reflectaFrames::sendEvent(Error, StackUnderflow);
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
  
  uint32_t microsBetweenFrames = 100000;
  
  void setFrameRate() {
    int16_t framesPerSecond = reflectaFunctions::pop16();
    microsBetweenFrames = 1000000 / framesPerSecond;
  };
  
  // Number of times loop is called and we're still waiting for one of our bound data
  // collection functions to finish
  uint16_t collectingLoops = 0;
  
  // Number of times loop is called and we've finished collecting data but the timeout
  // for next heartbeat has not yet expired
  uint16_t idleLoops = 0;
  
  void sendHeartbeat() {
    
      push16(idleLoops);
      push16(collectingLoops);
      push(Heartbeat);
      
      byte heartbeatSize = heartbeatStackTop + 1; 
      byte frame[heartbeatSize];
      
      for (int i = 0; i < heartbeatSize; i++)
      {
        frame[i] = pop();
      }
      
      reflectaFrames::sendFrame(frame, heartbeatSize);
  };
  
  bool finished = false;
  uint32_t nextHeartbeat = 0;
  void loop() {
    
    if (!finished) {
      collectingLoops++;
      finished = true;
      for (int i = 0; i < functionsTop; i++) {
        if (!functionComplete[i]) {
          functionComplete[i] = heartbeatFunctions[i]();
          if (!functionComplete[i]) finished = false;
        }
      }
    }
    
    if (finished) {
      
      unsigned long currentTime = micros();
      if (currentTime >= nextHeartbeat) {
        
        sendHeartbeat();
        
        // Set the micros delay for when the next heartbeat should be sent
        nextHeartbeat = currentTime + microsBetweenFrames;
        
        // Zero out the heartbeat state
        collectingLoops = 0;
        idleLoops = 0;
        
        for (int i = 0; i < MaxFunctions; i++) { 
          functionComplete[i] = false;
          finished = false;
        }
        
      } else {
        idleLoops++;
      };
    }
  };
  
  void setup() {
    reflectaFunctions::bind("hart1", setFrameRate);
  }
};
