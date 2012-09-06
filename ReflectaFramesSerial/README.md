__ReflectaFramesSerial__ packages byte[] data into frames over a stream, adds Sequence to detect lost frames, and adds Checksum to detect data corruption. Uses the Arduino Serial library for communications, a future Raw Hid implementation is planned.

It's not expected for people to build directly upon ReflectaFrames for anything more than using sendMessage() and sendError() for debugging purposes.  For most scenarios, working at the [ReflectaFunctions](https://github.com/JayBeavers/Reflecta/tree/master/ReflectaFunctions) (e.g. exposing an Arduino library up to a PC) or [ReflectaHeartbeat](https://github.com/JayBeavers/Reflecta/tree/master/ReflectaHeartbeat) (e.g. gathering data off the Arduino quickly and efficiently) level is a better choice.

## ReflectaFrames API

> _Stability: Medium_

```C
namespace reflectaFrames {

	// Setup the communications protocol, to be called in Arduino setup().
   	void setup(int speed);

	// Service the incoming communications data, to be called in Arduino loop().
   	void loop();

	// Send a frame of data returning the sequence id.
   	byte sendFrame(byte* frame, byte frameLength);

	// Send a string message, generally used for debugging
   	void sendMessage(String message);

	// Send a two byte frame notifying caller that something improper occured
	void sendEvent(FrameType type, byte code);

	// Function definition for the Frame Received function.
   	typedef void (*frameReceivedFunction)(byte sequence, byte frameLength, byte* frame);

	// Set the Frame Received Callback
   	void setFrameReceivedCallback(frameReceivedFunction frameReceived);

}
```

### Releases

- 0.3.x: Beta release of ReflectaFrames

## Silly Sample

```C
#include <ReflectaFramesSerial.h>

fine OHNOES 0x9A

void setup()
{
	reflectaFrames::setup(57600);
	reflectaFrames::setFrameReceivedCallback(frameReceived);
}

void loop()
{
	reflectaFrames::loop();
}

void frameReceived(byte sequence, byte length, byte* frame)
{
	reflectaFrames::sendError(OHNOES);

	reflectaFrames::sendMessage("I'm a lumberjack and I'm OK");

	byte buffer[2];
	buffer[0] = 98; // 0th byte is generally used to signal 'frame type'
	buffer[1] = 99;
	reflectaFrames::sendFrame(buffer, 2);
}
```

## Defined Constants

__Frame types__ -- sent in the 0th byte of the frame

```C
enum FrameType {
	Message   = 0x7D,
	Warning   = 0x7E,
	Error     = 0x7F
};
```

ReflectaFunctions defines 'function ids' for the 0th byte staring at 0x00 and incrementing upwards.  0x80 through 0xFF are reserved for 'user frame types'.

__Error / warning codes__ -- sent in the 1st byte of a frame type FrameType Warning or Error.

```C
enum EventCode {
	OutOfSequence     = 0x00,
	UnexpectedEscape  = 0x01,
	CrcMismatch       = 0x02,
	UnexpectedEnd     = 0x03,
	BufferOverflow    = 0x04
};
```

## Internals

Stuff in the code that's there for building higher level Reflecta libraries, subject to change.

```C
// Millisecond counter for last time a frame was received.  Can be used to implement a 'deadman switch' when
// communications with a host PC are lost or interrupted.
extern uint32_t lastFrameReceived;

// Reset the communications protocol (zero the sequence numbers & flush the communications buffers) 
void reset();

// Function definition for Frame Buffer Allocation function, to be optionally implemented by
// the calling library or application.
typedef byte (*frameBufferAllocationFunction)(byte** frameBuffer);

// Set the Buffer Allocation Callback
void setBufferAllocationCallback(frameBufferAllocationFunction frameBufferAllocation);
```

## Release History

- 0.3.x: Beta release ReflectaFrames.  Mature code, will only make changes based on community feedback.

## Futures

- May release a RawHID version of this protocol.
- May renumber FrameTypes