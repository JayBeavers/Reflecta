__ReflectaFramesSerial__ packages byte[] data into frames over a stream, adds Sequence to detect lost frames, and adds Checksum to detect data corruption. Uses the Arduino Serial library for communications, a future Raw Hid implementation is planned.

---

## ReflectaFrames API

	namespace reflectaFrames {

		// Setup the communications protocol, to be called in Arduino setup().
    	void setup(int speed);

		// Service the incoming communications data, to be called in Arduino loop().
    	void loop();

		// Send a frame of data returning the sequence id.
    	byte sendFrame(byte* frame, byte frameLength);

		// Send a string message.  
    	void sendMessage(String message);

    	// Send a two byte frame notifying caller that something improper occured.  
    	void sendError(byte eventId);

		// Function definition for the Frame Received function.
    	typedef void (*frameReceivedFunction)(byte sequence, byte frameLength, byte* frame);

		// Set the Frame Received Callback
    	void setFrameReceivedCallback(frameReceivedFunction frameReceived);

	}

---

## Silly Sample

    #include <ReflectaFramesSerial.h>

	#define OHNOES 0x9A

	void setup()
	{
  		reflectaFrames::setup(9600);
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

---

## Defined Constants

Frame types -- sent in the 0th byte of the frame

	FRAMES_MESSAGE                  	0x7D
	FRAMES_WARNING                  	0x7E
	FRAMES_ERROR                    	0x7F

Error/warning codes, sent in the 1st byte of a frame type FRAMES\_WARNING or FRAMES\_ERROR.

	FRAMES_WARNING_OUT_OF_SEQUENCE		0x00
	FRAMES_WARNING_UNEXPECTED_ESCAPE	0x01
	FRAMES_WARNING_CRC_MISMATCH			0x02
	FRAMES_WARNING_UNEXPECTED_END		0x03
	FRAMES_ERROR_BUFFER_OVERFLOW		0x04

---

## Internals

Stuff in the code that's there for building higher level Reflecta libraries, subject to change.


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
  