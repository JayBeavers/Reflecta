#include <Arduino.h>
#include <avr/wdt.h>

#ifndef _REFLECTA_H_
#define _REFLECTA_H_

namespace reflecta {

  const uint8_t kFrameSizeMax = 255;

  // Frame Ids used by Reflecta Functions.  These are reserved values for the
  // first byte of the frame data.
  enum FunctionId {
    PushArray           = 0x00,
    QueryInterface      = 0x01,
    SendResponse        = 0x02,
    SendResponseCount   = 0x03,
    Version             = 0x04,
    Reset               = 0x7A,
    Heartbeat           = 0x7B,
    Response            = 0x7C,
    Message             = 0x7D,
    Warning             = 0x7E,
    Error               = 0x7F
  };

  enum EventCode {
    OutOfSequence     = 0x00,
    UnexpectedEscape  = 0x01,
    CrcMismatch       = 0x02,
    UnexpectedEnd     = 0x03,
    BufferOverflow    = 0x04,
    FrameTooSmall     = 0x05,
    FunctionConflict  = 0x06,
    FunctionNotFound  = 0x07,
    ParameterMismatch = 0x08,
    StackOverflow     = 0x09,
    StackUnderflow    = 0x0A,
    WireNotAvailable  = 0x0B
  };

  void setup(int speed);
  void loop();

};  // namespace reflecta

namespace reflectaFrames {

  // Function definition for Frame Buffer Allocation function, to be optionally
  // implemented by the calling library or application.
  typedef uint8_t (*frameBufferAllocationFunction)(uint8_t** frameBuffer);

  // Function definition for the Frame Received function.
  typedef void (*frameReceivedFunction)(
    uint8_t sequence,
    uint8_t frameLength,
    uint8_t* frame);

  // Set the Frame Received Callback
  void setFrameReceivedCallback(frameReceivedFunction frameReceived);

  // Set the Buffer Allocation Callback
  void setBufferAllocationCallback(
    frameBufferAllocationFunction frameBufferAllocation);

  // Send a two byte frame notifying caller that something improper occured
  void sendEvent(reflecta::FunctionId type, uint8_t code);

  // Send a string message, generally used for debugging
  void sendMessage(const char* message);

  // Send a frame of data returning the sequence id
  uint8_t sendFrame(uint8_t* frame, uint8_t frameLength);

  // Reset the communications protocol (zero the sequence numbers & flush the
  // communications buffers)
  void reset();

  // Setup the communications protocol, to be called in Arduino setup()
  void setup(int speed);

  // Service the incoming communications data, to be called in Arduino loop()
  void loop();

  // Millisecond counter for last time a frame was received.  Can be used to
  // implement a 'deadman switch' when communications with a host PC are lost or
  // interrupted.
  extern uint32_t lastFrameReceived;
};  // namespace reflectaFrames

namespace reflectaFunctions {
  // Bind a function to an interfaceId so it can be called by Reflecta
  // Functions.  The assigned frame id is returned.
  uint8_t bind(const char* interfaceId, void (*function)());

  void push(int8_t b);

  void push16(int16_t b);

  int8_t pop();

  int16_t pop16();

  // Send a response to a function call
  //   callerSequence == the sequence number of the frame used to call the
  //     function used to correlate request/response on the caller side
  //   parameterLength & parameter byte* of the response data
  void sendResponse(uint8_t parameterLength, uint8_t* parameters);

  void setFirmwareVersion(const char* version);

  // reflectaFunctions setup() to be called in the Arduino setup() method
  void setup();

  // Execution pointer for Reflecta Functions.  To be used by functions that
  // change the order of instruction execution in the incoming frame.  Note:
  // if you are not implementing your own 'scripting language', you shouldn't
  // be using this.
  extern uint8_t* execution;

  // Top of the frame marker to be used when modifying the execution pointer.
  // Generally speaking execution should not go beyong frameTop.  When
  // execution == frameTop, the Reflecta Functions frameReceived execution loop
  // stops.
  extern uint8_t* frameTop;
};  // namespace reflectaFunctions

namespace reflectaArduinoCore {
  // ReflectaFunctions wrappers that receive the function call, parse the
  //   parameters, and call the matching Arduino libray functions
  void pinMode();
  void digitalRead();
  void digitalWrite();
  void analogRead();
  void analogWrite();

  void pulseIn();

  // Bind the Arduino core methods to the ARDU1 interface
  void setup();
};  // namespace reflectaArduinoCore

namespace reflectaHeartbeat {
  // How HB works:
  //  Has its own stack for gathering HB data
  //  Waits for fps timeout
  //  Calls array of functions asynchronously
  //    Each function returns true if done (remove, don't call again) or false
  //    for 'still running'
  //  When function array is empty, sends contents of hb stack
  //  Counts number of 'idle loops' when hb has been sent and fps timer hasn't
  //    expired
  //  Pushes 'idle loops' to the top of the stack

  // Returns the current state of isAlive
  bool alive();

  // Sets isAlive to true which is used to determine if the host is sending
  // a keepAlive signal to the Arduino
  void keepAlive();

  // Binds the setFrameRate function to QueryInterface
  void setup();

  // Gathers the HB data and sends it out when ready
  void loop();

  extern uint8_t* frameTop;

  void push(int8_t b);

  void push16(int16_t w);

  void pushf(float f);

  // Bind a function to an interfaceId so it can be called by Reflecta Functions
  // The assigned frame id is returned.
  void bind(bool (*function)());

  void setFrameRate();

};  // namespace reflectaHeartbeat

#endif  // _REFLECTA_H_
