/*
  ReflectaFunctions.cpp - Library for binding functions to a virtual function table
*/

#include <ReflectaFramesSerial.h>
#include "ReflectaFunctions.h"

namespace reflectaFunctions
{
  // Index of next unused function in the function table (vtable)
  byte openFunctionIndex = 2;
  
  // Function table that relates function id -> function
  void (*vtable[255])(byte sequence);
  
  // An interface is a well known group of functions.  Function id 0 == QueryInterface
  //   which allows a client to determine which functions an Arduino supports.
  // Maximum number of interfaces supported
  const byte maximumInterfaces = 25;

  // Number of interfaces defined
  byte indexOfInterfaces = 0;

  // Interface Id takes the form of CCCCIV
  //    CCCC is Company Id
  //    I is the Interface Id for the Company Id
  //    V is the Version Id for the Interface Id
  String interfaceIds[maximumInterfaces];

  // Interface starting function id, id of the first function in the interface
  //   in the vtable
  byte interfaceStart[maximumInterfaces];
  
  // Is this interface already defined?
  bool knownInterface(String interfaceId)
  {
    for(int index = 0; index < indexOfInterfaces; index++)
    {
      if (interfaceIds[index] == interfaceId)
      {
        return true;
      }
    }

    return false;
  }
  
  // Bind a function to the vtable so it can be remotely invoked.
  //   returns the function id of the binding in the vtable
  //   Note: You don't generally use the return value, the client uses
  //   QueryInterface (e.g. function id 0) to determine the function id
  //   remotely.
  byte bind(String interfaceId, void (*function)(byte sequence))
  {
    if (!knownInterface(interfaceId))
    {
      interfaceIds[indexOfInterfaces] = interfaceId;
      interfaceStart[indexOfInterfaces++] = openFunctionIndex;
    }
    
    if (vtable[openFunctionIndex] == NULL)
    {
      vtable[openFunctionIndex] = function;
    }
    else
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_FUNCTION_CONFLICT);
    }

    return openFunctionIndex++;
  }
  
  // Send a response frame to a function invoke.
  void sendResponse(byte callerSequence, byte parameterLength, byte* parameters)
  {
    byte frame[3 + parameterLength];
    
    frame[0] = FUNCTIONS_RESPONSE;
    frame[1] = callerSequence;
    frame[2] = parameterLength;
    memcpy(frame + 3, parameters, parameterLength);
    
    reflectaFrames::sendFrame(frame, 3 + parameterLength);
  }
  
  // Invoke the function, private method called by frameReceived
  void run(byte sequence, byte i)
  {
    if (vtable[i] != NULL)
    {
      vtable[i](sequence);
    }
    else
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_FUNCTION_NOT_FOUND);
    }
  }
  
  const byte parameterStackMax = 255;
  int parameterStackTop = -1;
  byte parameterStack[parameterStackMax + 1];
  
  void push(byte b)
  {
    if (parameterStackTop == parameterStackMax)
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_STACK_OVERFLOW);
    }
    else
    {
      parameterStack[++parameterStackTop] = b;
    }
  }
  
  byte pop()
  {
    if (parameterStackTop == -1)
      {
        reflectaFrames::sendError(FUNCTIONS_ERROR_STACK_UNDERFLOW);
      }
          else
    {
      return parameterStack[parameterStackTop--];
    }
  }
  
  reflectaFrames::frameReceivedFunction extendedParser = NULL;
  
  // Private function hooked to reflectaFrames to inspect incoming frames and
  //   Turn them into function calls.
  void frameReceived(byte sequence, byte frameLength, byte* frame)
  {
    if (frameLength > 0)
    {
        // TODO: Break frame into multiple function calls if present
        if (frameLength > 2 && frame[0] == FUNCTIONS_PUSHARRAY)
        {
          byte length = frame[1];
          for (int i = length + 1; i > 0; i--)
          {
            push(frame[i]);
          }
        }
        else if (frame[0] & 0x80 == 0x80) // extended parser
        {
          if (extendedParser != NULL)
            extendedParser(sequence, frameLength, frame);
        }
        else
        {
          run(sequence, frame[0]);
        }
    }
    else
    {
      reflectaFrames::sendError(FUNCTIONS_ERROR_FRAME_TOO_SMALL);
    }
  }
  
  // queryInterface is called by invoking function and passing as a
  //   parameter the interface id.
  // It returns the result by sending a response containing either the
  //    interface id of the first method or 0 if not found
  void queryInterface(byte sequence)
  {
    const byte parameterLength = 5;
    byte parameters[parameterLength];
    for (int i = 0; i < parameterLength; i++)
      parameters[i] = pop();
    
    for(int index = 0; index < indexOfInterfaces; index++)
    {
      String interfaceId = interfaceIds[index];
      
      // I could find no way to compare an Arduino String to byte* so I had to copy the
      //   String into a buffer and use strncmp
      char buffer[interfaceId.length() + 1];
      interfaceId.toCharArray(buffer, interfaceId.length() + 1);
            
      if (strncmp((const char*)buffer, (const char *)(parameters), parameterLength) == 0)
      {
        sendResponse(sequence, 1, interfaceStart + index);
        return;
      }
    }

    sendResponse(sequence, 1, 0);
  }

  void setup()
  {
    // Zero out the vtable function pointers
    memset(vtable, NULL, 255);
   
    // Bind the QueryInterface function in the vtable
    // Do this manually as we don't want to set a matching Interface
    vtable[FUNCTIONS_QUERYINTERFACE] = queryInterface;
    
    // TODO: block out FUNCTIONS_PUSHARRAY, FRAMES_ERROR, FRAMES_MESSAGE, and FUNCTIONS_RESPONSE too
    
    // Hook the incoming frames and turn them into function calls
    reflectaFrames::setFrameReceivedCallback(frameReceived);
  }
  
  void setExtendedParser(reflectaFrames::frameReceivedFunction fn)
  {
    extendedParser = fn;
  }

};
