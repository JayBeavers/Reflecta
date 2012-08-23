/*
ReflectaFramesSerial.h - Library for sending frames of information from a Microcontroller to a PC over a serial port.
*/

#include <Arduino.h>

#ifndef REFLECTA_FRAMES_H
#define REFLECTA_FRAMES_H

namespace reflectaFrames
{
  enum FrameType {
    Message   = 0x7D,
    Warning   = 0x7E,
    Error     = 0x7F
  };

  enum EventCode {
    OutOfSequence     = 0x00,
    UnexpectedEscape  = 0x01,
    CrcMismatch       = 0x02,
    UnexpectedEnd     = 0x03,
    BufferOverflow    = 0x04
  };

  // Function definition for Frame Buffer Allocation function, to be optionally implemented by
  // the calling library or application.
  typedef byte (*frameBufferAllocationFunction)(byte** frameBuffer);
  
  // Function definition for the Frame Received function.
  typedef void (*frameReceivedFunction)(byte sequence, byte frameLength, byte* frame);
  
  // Set the Frame Received Callback
  void setFrameReceivedCallback(frameReceivedFunction frameReceived);
  
  // Set the Buffer Allocation Callback
  void setBufferAllocationCallback(frameBufferAllocationFunction frameBufferAllocation);
  
  // Send a two byte frame notifying caller that something improper occured
  void sendEvent(FrameType type, byte code);
  
  // Send a string message, generally used for debugging
  void sendMessage(String message);
  
  // Send a frame of data returning the sequence id
  byte sendFrame(byte* frame, byte frameLength);
  
  // Reset the communications protocol (zero the sequence numbers & flush the communications buffers) 
  void reset();
  
  // Setup the communications protocol, to be called in Arduino setup()
  void setup(int speed);
  
  // Service the incoming communications data, to be called in Arduino loop()
  void loop();
  
  // Millisecond counter for last time a frame was received.  Can be used to implement a 'deadman switch' when
  // communications with a host PC are lost or interrupted.
  extern uint32_t lastFrameReceived;
};

#endif