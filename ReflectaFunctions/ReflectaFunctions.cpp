/*
  ReflectaFunctions.cpp - Library for binding functions to a virtual function table
*/

#include "ReflectaFunctions.h"

using namespace reflectaFrames;

namespace reflectaFunctions
{
  // Index of next unused function in the function table (vtable)
  byte openFunctionIndex = 1;
  
  // Function table that relates function id -> function
  void (*vtable[255])(byte callerSequence, byte parameterLength, byte* parameters);
  
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
  byte   interfaceStart[maximumInterfaces];
  
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
  byte bind(String interfaceId, void (*function)(byte callerSequence, byte parameterLength, byte* parameters))
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
      sendError(FUNCTIONS_ERROR_FUNCTION_CONFLICT);
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
    
    sendFrame(frame, 3 + parameterLength);
  }
  
  // Invoke the function, private method called by frameReceived
  void run(byte callerSequence, byte i, byte parameterLength, byte* parameters)
  {
    if (vtable[i] != NULL)
    {
      vtable[i](callerSequence, parameterLength, parameters);
    }
    else
    {
      sendError(FUNCTIONS_ERROR_FUNCTION_NOT_FOUND);
    }
  }
  
  // Private function hooked to reflectaFrames to inspect incoming frames and
  //   Turn them into function calls.
  void frameReceived(byte sequence, byte frameLength, byte* frame)
  {
    if (frameLength > 2)
    {
      // TODO: Break frame into multiple function calls if present
      
      // First byte of frame is function id, rest of frame is function parameter
      run(sequence, frame[0], frame[1], frame + 2);
    }
    else if (frameLength > 0)
    {      
      // First byte of frame is function id, rest of frame is function parameter
      run(sequence, frame[0], NULL, NULL);
    }
    else
    {
      sendError(FUNCTIONS_ERROR_FRAME_TOO_SMALL);
    }
  }
  
  // queryInterface is called by invoking function id 0 and passing as a
  //   parameter the interface id.
  // It returns the result by sending a response containing either the
  //    interface id of the first method or 0 if not found
  void queryInterface(byte sequence, byte parameterLength, byte* parameters)
  {
    //    255 SEQ CCCCIV#CCCCIV#CCCCIV#
    //      255 says 'this is a response to'
    //      SEQ is the SEQ of the frame that issued the QueryInterface call
  
    //    CCCCIV an Interface per above
    //    # is the 'function pointer offset' where the interface's functions are in the vtable
  
    //    Since our packet size is constrained to 64 bytes and we may overflow the packet, we return at most five interfaces per frame
    //    QueryInterface response is hardcoded in source code

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
    // TODO: hook FRAMES_ERROR, FRAMES_MESSAGE, and FUNCTIONS_RESPONSE too
    
    // Hook the incoming frames and turn them into function calls
    setFrameReceivedCallback(frameReceived);
  }
};
