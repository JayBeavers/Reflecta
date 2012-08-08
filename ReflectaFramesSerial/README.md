# ReflectaFrames #

Setup the communications protocol, to be called in Arduino setup().
> void setup(int speed);

Send a frame of data returning the sequence id.
> byte sendFrame(byte* frame, byte frameLength);

Send a string message.  frame[0] = 0xFE is used to denote 'string message' followed by frame[1] = string length, then frame[2...] = UTF8 bytes.
> void sendMessage(String message);

Send a two byte frame notifying caller that something improper occured.  frame[0] = 0xFF is used to denote 'error', followed by frame[1] = one byte error code.
> void sendError(byte eventId);

Function definition for the Frame Received function.
> typedef void (*frameReceivedFunction)(byte sequence, byte frameLength, byte* frame);

Set the Frame Received Callback in your Arduino program.
> void setFrameReceivedCallback(frameReceivedFunction frameReceived);

Service the incoming communications data, to be called in Arduino loop().
> void loop();
