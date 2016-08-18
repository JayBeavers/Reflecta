/*
ReflectaFramesSerial.cpp - Library for sending frames of information from a Microcontroller to a PC over a serial port.
*/

#include "Reflecta.h"

namespace reflectaFrames {
  // SLIP (http://www.ietf.org/rfc/rfc1055.txt) protocol special character
  // definitions used to find end of frame when using a streaming communications
  // protocol
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

  // Checksum for the incoming frame, calculated byte by byte using XOR.
  // Compared against the checksum byte which is stored in the last byte of the
  // frame.
  uint8_t readChecksum = 0;

  // Checksum for the outgoing frame, calculated byte by byte using XOR.  Added
  // to the payload as the last byte of the frame.
  uint8_t writeChecksum = 0;

  // Sequence number of the incoming frames.  Compared against the sequence
  // number at the beginning of the incoming frame to detect out of sequence
  // frames which would point towards lost data or corrupted data.
  uint8_t readSequence = 0;

  // Sequence number of the outgoing frames.
  uint8_t writeSequence = 0;

  // protocol parser escape state -- set when the ESC character is detected so
  // the next character will be de-escaped
  bool escaped = false;

  // protocol parser state
  int state = FrameStart;

  frameBufferAllocationFunction frameBufferAllocationCallback = NULL;
  frameReceivedFunction frameReceivedCallback = NULL;

  void setFrameReceivedCallback(frameReceivedFunction frameReceived) {
    frameReceivedCallback = frameReceived;
  }

  void setBufferAllocationCallback(
    frameBufferAllocationFunction frameBufferAllocation) {
    frameBufferAllocationCallback = frameBufferAllocation;
  }

  void writeEscaped(uint8_t b) {
    switch (b) {
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

  uint8_t sendFrame(uint8_t* frame, uint8_t frameLength) {
    writeChecksum = 0;
    writeEscaped(writeSequence);
    for (uint8_t index = 0; index < frameLength; index++) {
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

  void sendEvent(reflecta::FunctionId type, uint8_t code) {
    uint8_t buffer[2];
    buffer[0] = type;
    buffer[1] = code;
    sendFrame(buffer, 2);
  }

  void sendMessage(const char* message) {
    uint8_t messageLength = strlen(message);
    char buffer[reflecta::kFrameSizeMax];

    if (2 + messageLength > reflecta::kFrameSizeMax) {
      reflectaFrames::sendEvent(reflecta::Error, reflecta::BufferOverflow);
      return;
    }

    buffer[0] = reflecta::Message;
    buffer[1] = messageLength;
    memcpy(buffer + 2, message, messageLength);

    // Strip off the trailing '\0' that Arduino String.getBytes insists on
    // postpending
    sendFrame((uint8_t*)buffer, messageLength + 2);
  }

  int readUnescaped(uint8_t *b) {
    *b = Serial.read();

    if (escaped) {
      switch (*b) {
        case EscapedEnd:
          *b = End;
          break;
        case EscapedEscape:
          *b = Escape;
          break;
        default:
          sendEvent(reflecta::Warning, reflecta:: UnexpectedEscape);
          state = FrameInvalid;
          break;
      }
      escaped = false;
      readChecksum ^= *b;
    } else {
      if (*b == Escape) {
        escaped = true;
        return 0;  // read escaped value on next pass
      }
      if (*b == End) {
        switch (state) {
          case FrameInvalid:
            readChecksum = 0;
            state = FrameStart;
            break;
          case ReadingFrame:
            state = FrameEnded;
            break;
          default:
            sendEvent(reflecta::Warning, reflecta::UnexpectedEnd);
            state = FrameInvalid;
            break;
        }
      } else {
        readChecksum ^= *b;
      }
    }

    return 1;
  }

  const uint8_t frameBufferSourceLength = 64;
  uint8_t* frameBufferSource = NULL;

  // Default frame buffer allocator for when caller does not set one.
  uint8_t frameBufferAllocation(uint8_t** frameBuffer) {
    *frameBuffer = frameBufferSource;
    return frameBufferSourceLength;
  }

  void reset() {
    readSequence = 0;
    writeSequence = 0;
    Serial.flush();
  }

  void setup(int speed) {
    if (frameBufferAllocationCallback == NULL) {
      frameBufferSource = reinterpret_cast<uint8_t*>(
        malloc(frameBufferSourceLength));
      frameBufferAllocationCallback = frameBufferAllocation;
    }

    Serial.begin(speed);
    Serial.flush();
  }

  uint8_t* frameBuffer;
  uint8_t frameBufferLength;
  uint8_t frameIndex = 0;

  uint8_t sequence;

  uint32_t lastFrameReceived;

  void loop() {
    uint8_t b;

    while (Serial.available()) {
      if (readUnescaped(&b)) {
        switch (state) {
          case FrameInvalid:
            break;

          case FrameStart:
            sequence = b;
            if (readSequence++ != sequence) {
              // Only send an out of sequence warning if the time between frames
              // is < 10 seconds.  This is because we have no 'port opened/port
              // closed' API on Arduino to tell when a connection has been
              // physically reset by the host
              if (lastFrameReceived - millis() < 10000) {
                char message[30];
                snprintf(message, 30, "Expected %x received %x", readSequence - 1, sequence);
                sendMessage(message);
                sendEvent(reflecta::Warning, reflecta::OutOfSequence);
              }

              readSequence = sequence + 1;
            }

            frameBufferLength = frameBufferAllocationCallback(&frameBuffer);
            frameIndex = 0;  // Reset the buffer pointer to beginning
            readChecksum = sequence;
            state = ReadingFrame;
            break;

          case ReadingFrame:
            if (frameIndex == frameBufferLength) {
              sendEvent(reflecta::Error, reflecta::BufferOverflow);
              state = FrameInvalid;
              readChecksum = 0;
            } else {
              frameBuffer[frameIndex++] = b;
            }
            break;

          case FrameEnded:
            lastFrameReceived = millis();
            // zero expected because finally XOR'd with itself
            if (readChecksum == 0) {
              // TODO(jay): add a MessageReceived callback too

              if (frameReceivedCallback != NULL) {
                frameReceivedCallback(sequence, frameIndex - 1, frameBuffer);
              }
            } else {
              sendEvent(reflecta::Warning, reflecta::CrcMismatch);
              state = FrameInvalid;
              readChecksum = 0;
            }
            state = FrameStart;
            break;
        }
      }
    }
  }
}  // namespace reflectaFrames
