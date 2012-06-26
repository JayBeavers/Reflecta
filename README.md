# Reflecta #

Reflecta is a remote communications library for Arduino.  It enables you to integrate Arduino with your host computer applications by remotely calling Arduino functions like digitalWrite or analogRead, writing Arduino programs that send & receive frames of byte data or string messages, or receiving high speed GPIO data.

## Why? ##

The motivation to write Reflecta came from a need to combine remote IO and remote function calling for robotics scenarios.  A host PC can read sensor data off the Arduino at 400+ frames per second while calling Arduino libraries or applications to perform time sensitive (e.g. real time) tasks such as servo control, PWM, amd I2C communications.

## How? ##

Reflecta is made of five small libraries that work together:

- ReflectaFramesSerial.h -- a framing protocol that packages byte[] data into individual frames and adds support for tracking Sequence, detecting errors, and detecting the end of the frame.  It it roughly based on [SLIP](http://tools.ietf.org/html/rfc1055) for framing and the [STK500 communications protocol](http://www.atmel.com/Images/doc2591.pdf) for packet format.  Initial implementation (ReflectaFramesSerial) uses the Arduino Serial library for communications, a future Raw Hid implementation is planned.

- ReflectaFunctions.h -- a remote function calling protocol that builds on top of ReflectaFrames.  The first byte of each frame is 'function id', the second byte is 'parameter length', followed by byte[] parameters.  Arduino functions are 'registered' for rpc by calling bind(Interface, Function Pointer).  ReflectaFunctions defines function '0' as 'QueryInterface' which is used to determine whether this firmware implements an Interface.

- ReflectaArduinoCore.h -- a binding of the Arduino core library functions such as pinMode, digitalRead, analogWrite to the 'ARDU1' interface.

- ReflectaHeartbeat.h -- an Arduino-side library for reading digital and analog pins very efficienctly and sending the results at a fixed frequency to the host PC.

----------

## ReflectaFrames ##

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

----------

## ReflectaFunctions ##

Bind a function to an interfaceId so it can be called by Reflecta Functions.  Function receives the Sequence number of the calling frame (to be used when sending a response) and the byte* parameters.
>  byte bind(String interfaceId, void (*function)(byte callerSequence, byte parameterLength, byte* parameters));

Send a response to a function call.  Match the sequence of your caller to allow the host computer to correlate the request & response.
> void sendResponse(byte callerSequence, byte parameterLength, byte* parameters);

reflectaFunctions setup() to be called in the Arduino setup() method.
> void setup();

To remotely call an Arduino function, send a frame consisting of:

frame[0] = functionId

frame[1] = length of parameter data

frame[2...] = parameter data

For example, to invoke function id 7 with parameter data 'ABC', send:

> 0x07 03 65 66 67

> FunctionId = 7, ParameterLength = 3, A = 65, B = 66, C = 67

To QueryInterface to find out if interface 'ARDU1' (Arduino Core Library version 1) is implemented and if so where its function index starts, send a frame to function id '0' with parameters of 'ARDU1'.

> 0x00 05 65 82 68 85 49

> Function Id = 0 (QueryInterface), ParameterLength = 5, A = 65, R = 82, D = 68, U = 85, 1 = 49

This will return a response packet containing the sequence of the calling frame.  Response packets have frame[0] = 0xFD.  For example, if the calling frame was sequence 99 and the ARDU1 interface functions start at function id 6, the response frame coming into the host PC would contain:

> 0xFD 99 01 06

> Function Id = 0xFD (Response), SEQ = 99, ParameterLength = 1, Function Index = 6

It's assumed that a higher level Reflecta Functions client library will hide this detail from the caller, instead exposing a simple function API that uses QueryInterface and sendFrame behind the scenes to make function calls appear local.

----------

## ReflectaArduinoCore ##

Reflecta Arduino Core binds the functions pinMode, digitalRead, digitalWrite, analogRead, and analogWrite to the ARDU1 interface.  To use, QueryInterface 'ARDU1' and this will give you the function id of pinMode.  Increase the function id for each following function in the interface.  For example, if QueryInterface ARDU1 returns 9, then function id 9 = pinMode and function id 12 = analogRead.

pinMode, digitalWrite, and analogWrite take two byte parameters, one byte for each function parameter.  digitalRead and analogRead take one byte parameters.  For example, when QueryInterface ARDU1 = 9, sending

> 0x09 02 11 1

> Function Id = 0x09, ParameterLength = 2, Parameter[0] = 11, Parameter[1] = 1

would call

> pinMode(11, 1);