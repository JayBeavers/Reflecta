/*
ReflectaFramesSerial.cpp - Library for sending frames of information from a Microcontroller to a PC over a serial port.
*/

#include "ReflectaFramesSerial.h"

namespace reflectaFrames
{
  // SLIP (http://www.ietf.org/rfc/rfc1055.txt) protocol special character definitions
  // Used to find end of frame when using a streaming communications protocol
  enum EscapeCharacters {
    End           = 0xC0,
    Escape        = 0xDB,
    EscapedEnd    = 0xDC,
    EscapedEscape = 0xDD
  };

  enum ReadState {
    FrameStart,
    ReadingFrame,
    FrameEnded,
    FrameInvalid
  };

  // Checksum for the incoming frame, calculated byte by byte using XOR.  Compared against the checksum byte
  // which is stored in the last byte of the frame.
  byte readChecksum = 0;
  
  // Checksum for the outgoing frame, calculated byte by byte using XOR.  Added to the payload as the last byte of the frame. 
  byte writeChecksum = 0;
  
  // Sequence number of the incoming frames.  Compared against the sequence number at the beginning of the incoming frame
  //  to detect out of sequence frames which would point towards lost data or corrupted data.
  byte readSequence = 0;
  
  // Sequence number of the outgoing frames.
  byte writeSequence = 0;
  
  // protocol parser escape state -- set when the ESC character is detected so the next character will be de-escaped
  bool escaped = false;
  
  // protocol parser state
  int state = FrameStart;
  
  frameBufferAllocationFunction frameBufferAllocationCallback = NULL;
  frameReceivedFunction frameReceivedCallback = NULL;
  
  void setFrameReceivedCallback(frameReceivedFunction frameReceived)
  {
    frameReceivedCallback = frameReceived;
  }
  
  void setBufferAllocationCallback(frameBufferAllocationFunction frameBufferAllocation)
  {
    frameBufferAllocationCallback = frameBufferAllocation;
  }
  
  void writeEscaped(byte b)
  {
    switch(b)
    {
      case End:
        Serial.write(Escape);
        Serial.write(EscapedEnd);
        break;
      case Escape:
        Serial.write(Escape);
        Serial.write(EscapedEscape);
        break;
      default:
        Serial.write(b);
        break;
    }
    writeChecksum ^= b;
  }
  
  byte sendFrame(byte* frame, byte frameLength)
  {
    writeChecksum = 0;
    writeEscaped(writeSequence);
    for (byte index = 0; index < frameLength; index++)
    {
      writeEscaped(frame[index]);
    }
    writeEscaped(writeChecksum);
    Serial.write(End);

    // On Teensies, use the extended send_now to perform an undelayed send
    #ifdef USBserial_h_
    Serial.send_now();
    #endif
    
    return writeSequence++;
  }
  
  void sendEvent(FrameType type, byte code)
  {
    byte buffer[2];
    buffer[0] = type;
    buffer[1] = code;
    sendFrame(buffer, 2);
  }
  
  void sendMessage(String message)
  {
    byte bufferLength = message.length() + 3;
    byte buffer[bufferLength];
    
    buffer[0] = Message;
    buffer[1] = message.length();
    message.getBytes(buffer + 2, bufferLength - 2);
    
    // Strip off the trailing '\0' that Arduino String.getBytes insists on postpending
    sendFrame(buffer, bufferLength - 1);
  }
  
  int readUnescaped(byte &b)
  {
    b = Serial.read();
    
    if (escaped)
    {
      switch (b)
      {
        case EscapedEnd:
          b = End;
          break;
        case EscapedEscape:
          b = Escape;
          break;
        default:
          sendEvent(Warning, UnexpectedEscape);
          state = FrameInvalid;
          break;
      }
      escaped = false;
      readChecksum ^= b;
    }
    else
    {
      if (b == Escape)
      {
        escaped = true;
        return 0; // read escaped value on next pass
      }
      if (b == End)
      {
        switch (state)
        {
          case FrameInvalid:
            readChecksum = 0;
            state = FrameStart;
            break;
          case ReadingFrame:
            state = FrameEnded;
            break;
          default:
            sendEvent(Warning, UnexpectedEnd);
            state = FrameInvalid;
            break;
        }
      }
      else
      {
        readChecksum ^= b;
      }
    }
    
    return 1;
  }
  
  const byte frameBufferSourceLength = 64;
  byte* frameBufferSource = NULL;
  
  // Default frame buffer allocator for when caller does not set one.
  byte frameBufferAllocation(byte** frameBuffer)
  {
    *frameBuffer = frameBufferSource;
    return frameBufferSourceLength;
  }
  
  void reset()
  {
    readSequence = 0;
    writeSequence = 0;
    Serial.flush();
  }
  
  void setup(int speed)
  {
    if (frameBufferAllocationCallback == NULL)
    {
      frameBufferSource = (byte*)malloc(frameBufferSourceLength);
      frameBufferAllocationCallback = frameBufferAllocation;
    }
    
    Serial.begin(speed);
    Serial.flush();
  }
  
  byte* frameBuffer;
  byte frameBufferLength;
  byte frameIndex = 0;

  byte sequence;

  uint32_t lastFrameReceived;
  
  void loop()
  {
    byte b;
    
    while (Serial.available())
    {
      if (readUnescaped(b))
      {
        switch (state)
        {
          case FrameInvalid:
            break;
            
          case FrameStart:
            sequence = b;
            if (readSequence++ != sequence)
            {
              sendMessage("Expected " + String(readSequence - 1, HEX) + " received " + String(sequence, HEX) );
              readSequence = sequence + 1;
              sendEvent(Warning, OutOfSequence);
            }
            
            frameBufferLength = frameBufferAllocationCallback(&frameBuffer);
            frameIndex = 0; // Reset the buffer pointer to beginning
            readChecksum = sequence;
            state = ReadingFrame;
            break;
            
          case ReadingFrame:
            if (frameIndex == frameBufferLength)
            {
              sendEvent(Error, BufferOverflow);
              state = FrameInvalid;
              readChecksum = 0;
            }
            else
            {
              frameBuffer[frameIndex++] = b;
            }
            break;
            
          case FrameEnded:
            lastFrameReceived = millis();
            if (readChecksum == 0) // zero expected because finally XOR'd with itself
            {
              // TODO: add a MessageReceived callback too
              
              if (frameReceivedCallback != NULL)
              {
                frameReceivedCallback(sequence, frameIndex - 1, frameBuffer);
              }
            }
            else
            {
              sendEvent(Warning, CrcMismatch);
              state = FrameInvalid;
              readChecksum = 0;
            }
            state = FrameStart;
            break;
        }
      }
    }
  }
}