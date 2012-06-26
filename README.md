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

Setup the communications protocol, to be called in Arduino setup()
> void setup(int speed);

Send a frame of data returning the sequence id
> byte sendFrame(byte* frame, byte frameLength);

Send a string message
> void sendMessage(String message);

Send a two byte frame notifying caller that something improper occured
> void sendError(byte eventId);

Function definition for the Frame Received function.
> typedef void (*frameReceivedFunction)(byte sequence, byte frameLength, byte* frame);

Set the Frame Received Callback
> void setFrameReceivedCallback(frameReceivedFunction frameReceived);

Service the incoming communications data, to be called in Arduino loop()
> void loop(); // Service the incoming communications data, to be called in Arduino loop()