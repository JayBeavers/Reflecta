/*
ReflectaFunctions.cpp - Library for binding functions to a virtual function table
*/

#include "Reflecta.h"

namespace reflectaFunctions {

  // Version of this firmware
  char* firmwareVersion = "reflecta-2015.10.08.01";

  // Index of next unused function in the function table (vtable)
  uint8_t openFunctionIndex = 5;

  // Function table that relates function id -> function
  void (*vtable[255])();

  // An interface is a well known group of functions.  Function
  // id 0 == QueryInterface which allows a client to determine which functions
  // an Arduino supports.
  // Maximum number of interfaces supported
  const uint8_t kMaximumInterfaces = 25;
  const uint8_t kMaximumInterfaceSize = 6;

  // Number of interfaces defined
  uint8_t indexOfInterfaces = 0;

  // Interface Id takes the form of CCCCIV
  //    CCCC is Company Id
  //    I is the Interface Id for the Company Id
  //    V is the Version Id for the Interface Id
  char interfaceIds[kMaximumInterfaces][kMaximumInterfaceSize];

  // Interface starting function id, id of the first function in the interface
  //   in the vtable
  uint8_t interfaceStart[kMaximumInterfaces];

  void setFirmwareVersion(char* version) {
    firmwareVersion = version;
  }

  // Is this interface already defined?
  bool knownInterface(char* interfaceId) {
    for (int index = 0; index < indexOfInterfaces; index++) {
      if (strncmp(interfaceIds[index], interfaceId, kMaximumInterfaceSize) == 0) {
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
  uint8_t bind(char* interfaceId, void (*function)()) {
    if (!knownInterface(interfaceId)) {
      strncpy(interfaceIds[indexOfInterfaces], interfaceId, kMaximumInterfaceSize);
      interfaceStart[indexOfInterfaces++] = openFunctionIndex;
    }

    if (vtable[openFunctionIndex] == NULL) {
      vtable[openFunctionIndex] = function;
    } else {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::FunctionConflict);
    }

    return openFunctionIndex++;
  }

  uint8_t callerSequence;

  // Send a response frame from a function invoke.  Used when the function
  // automatically returns data to the caller.
  void sendResponse(uint8_t parameterLength, uint8_t* parameters) {
    uint8_t frame[reflecta::kFrameSizeMax];

    if (3 + parameterLength > reflecta::kFrameSizeMax) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::BufferOverflow);
      return;
    }

    frame[0] = reflecta::Response;
    frame[1] = callerSequence;
    frame[2] = parameterLength;
    memcpy(frame + 3, parameters, parameterLength);

    reflectaFrames::sendFrame(frame, 3 + parameterLength);
  }

  // Invoke the function, private method called by frameReceived
  void run(uint8_t i) {
    if (vtable[i] != NULL) {
      vtable[i]();
    } else {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::FunctionNotFound);
    }
  }

  const uint8_t kParameterStackMax = 128;
  int parameterStackTop = -1;
  int8_t parameterStack[kParameterStackMax + 1];

  void push(int8_t b) {
    if (parameterStackTop == kParameterStackMax) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::StackOverflow);
    } else {
      parameterStack[++parameterStackTop] = b;
    }
  }

  void push16(int16_t w) {
    if (parameterStackTop > kParameterStackMax - 2) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::StackOverflow);
    } else {
      parameterStackTop += 2;
      *(reinterpret_cast<int16_t*>(parameterStack + parameterStackTop - 1)) = w;
    }
  }

  int8_t pop() {
    if (parameterStackTop == -1) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::StackUnderflow);
      return -1;
    } else {
      return parameterStack[parameterStackTop--];
    }
  }

  int16_t pop16() {
    if (parameterStackTop == -1 || parameterStackTop == 0) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::StackUnderflow);
      return -1;
    } else {
      parameterStackTop -= 2;
      return
        *(reinterpret_cast<int16_t*>(parameterStack + parameterStackTop + 1));
    }
  }

  // Request a response frame from data that is on the parameterStack.  Used to
  // retrieve a count of 'n' data bytes that were push on the parameterStack
  // from a previous invocation.  The count of bytes to be returned is
  // determined by popping a byte off the stack so it's expected that
  // 'PushArray 1 ResponseCount' is called first.
  void sendResponseCount() {
    int8_t count = pop();
    uint8_t size = 3 + count;

    uint8_t frame[reflecta::kFrameSizeMax];

    if (3 + count > reflecta::kFrameSizeMax) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::BufferOverflow);
      return;
    }

    frame[0] = reflecta::Response;
    frame[1] = callerSequence;
    frame[2] = count;
    for (int i = 0; i < count; i++) {
      int8_t value = pop();
      frame[3 + i] = value;
    }

    reflectaFrames::sendFrame(frame, size);
  }

  // Request a response frame of one byte data that is on the parameterStack.
  // Used to retrieve data pushed on the parameterStack from a previous function
  // invocation.
  void sendResponse() {
    push(1);
    sendResponseCount();
  }

  // Execution pointer for Reflecta Functions.  To be used by functions that
  // change the order of instruction execution in the incoming frame.  Note:
  // if you are not implementing your own 'scripting language', you shouldn't
  // be using this.
  uint8_t* execution;

  // Top of the frame marker to be used when modifying the execution pointer.
  // Generally speaking execution should not go beyond frameTop.  When
  // execution == frameTop, the Reflecta Functions frameReceived execution loop
  // stops.
  uint8_t* frameTop;

  void pushArray() {
    // Pull off array length
    if (execution == frameTop) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::FrameTooSmall);
    }
    uint8_t length = *execution++;

    // Push array data onto parameter stack as bytes, reversed
    // Do not include the length when pushing, just the data
    for (int i = length - 1; i > -1; i--) {
      push(*(execution + i));
    }

    // Increment the execution pointer past the data array, being careful not to
    // exceed the frame size
    for (int i = 0; i < length; i++) {
      execution++;
      if (execution > frameTop) {
        reflectaFrames::sendEvent(reflecta::Error, reflecta::FrameTooSmall);
        break;
      }
    }
  }

  // Private function hooked to reflectaFrames to inspect incoming frames and
  //   Turn them into function calls.
  void frameReceived(byte sequence, byte frameLength, byte* frame) {
    execution = frame;  // Set the execution pointer to the start of the frame
    callerSequence = sequence;
    frameTop = frame + frameLength;

    while (execution != frameTop) {
      run(*execution++);
    }
  }

  // queryInterface is called by invoking function.  It returns a response
  // packet containing the interface id and start index of each registered
  // interface
  void queryInterface() {
    const int interfaceIdLength = 5;

    for (int index = 0; index < indexOfInterfaces; index++) {
      for (int stringIndex = interfaceIdLength - 1;
        stringIndex > -1;
        stringIndex--) {
        push(interfaceIds[index][stringIndex]);
      }
      push(interfaceStart[index]);
    }

    // each interface contributes 1 payload byte for startIndex and 'n' bytes
    // for the interfaceId string
    push((interfaceIdLength + 1) * indexOfInterfaces);
    sendResponseCount();
  }

  void version() {
    sendResponse(strlen(firmwareVersion), (uint8_t*)firmwareVersion);
  }

  void reset() {
    parameterStackTop = -1;
    reflectaFrames::reset();
  }

  void setup() {
    // Zero out the vtable function pointers
    memset(vtable, NULL, 255);

    // Bind the QueryInterface function in the vtable
    // Do this manually as we don't want to set a matching Interface
    vtable[reflecta::PushArray] = pushArray;
    vtable[reflecta::QueryInterface] = queryInterface;
    vtable[reflecta::SendResponse] = sendResponse;
    vtable[reflecta::SendResponseCount] = sendResponseCount;
    vtable[reflecta::Version] = version;
    vtable[reflecta::Reset] = reset;

    // TODO(jay): block out FRAMES_ERROR, FRAMES_MESSAGE, and FUNCTIONS_RESPONSE
    // too

    // Hook the incoming frames and turn them into function calls
    reflectaFrames::setFrameReceivedCallback(frameReceived);
  }
};  // namespace reflectaFunctions
