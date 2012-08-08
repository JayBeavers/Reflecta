# Reflecta #

Reflecta is a remote communications library for Arduino.  It enables you to integrate Arduino with your host computer by remotely calling Arduino functions like digitalWrite or analogRead, writing Arduino programs that send & receive frames of byte data or string messages, and receiving high speed (400+ frames per second) GPIO 'heartbeats'.  This enables you to use Arduino libraries to talk to a wide variety of hardware, run time sensitive operations like PWM or Pulse In / Pulse Out (e.g. servos, sonars) on the Arduino, and use the Arduino as a bridge to I2C and SPI devices.  This gives us the best of all worlds -- we can leverage the high speeds of x86 and ARM CPUs to perform heavy lifting like sensor fusion and data filtering while using the Arduino to handle microsecond sensitive tasks.

## Why? ##

Arduinos easily connect to a PC over USB as a USB Virtual COM port but turning this serial stream into a reliable communications channel takes considerable work.  

At first I used Firmata -- a built-in library for GPIO remoting and Wire bridging.  However I found some limitations with this approach:

- Firmata, based on MIDI, uses a '7 bit data' design.  The 8th bit is reserved for 'commands' so all data sent over the wire has to be converted to/from 7 bits.

- Firmata is difficult to extend with new methods due to its SYSEX design and 7-bitness.

- Firmata doesn't detect corrupt or lost data.  Testing using USB Virtual COM with two-chip boards like the UNO detected frequent data corruption at speeds over 9600 baud.  RF communications like Bluetooth or Zigbee need to be able to detect data loss and corruption in order to modify their sending behavior.

The goal is to replace Firmata with a protocol that fixes these issues, specifically:

- Use 'escaping' to delineate frames so we don't need to convert payload data to 7 bits.

- Add Sequence and Checksum to detect data loss and corruption.

- Make it easy to extend the protocol with new functions (bind) and discover what functions are implemented (queryinterface).

- Design for CPU and communications efficiency in order to take best advantage of limited microcontroller resources.

### Frame Protocol ###

After reviewing existing technologies, the design approach settled on is:

- Start with the [STK500 protocol](http://www.atmel.com/Images/doc2591.pdf) from Atmel which has MESSAGE_START, SEQUENCE_NUMBER, and a simple 8 bit xor CHECKSUM.
- STK500 doesn't escape the data, so substitute [SLIP framing](http://www.ietf.org/rfc/rfc1055.txt) for the STK500 message start/message size design.  SLIP is very simple to understand and code.

A frame of data ends up looking like:

SEQUENCE Payload[] CHECKSUM END

### Escape Protocol (SLIP, e.g. Serial Line IP) ###

SLIP escaping adds approximately 1% overhead to binary data.  It defines two special characters, END(0xC0) and ESCAPE(0xDB).

When sending a frame of data,

- Replace Payload END (0xC0) with ESCAPE (0xDB) + ESCAPED_END (0xDC)
- Replace Payload ESCAPE (0xDB) with ESCAPE (0xDB) + ESCAPED_ESCAPE (0xDD)
- Write END (0xC0) and the end of payload

## How? ##

Reflecta is made of four small libraries that work together:

- ReflectaFramesSerial -- a framing protocol that packages byte[] data into individual frames over a data stream and adds support for tracking Sequence, detecting errors, and detecting the end of the frame. Initial implementation (ReflectaFramesSerial) uses the Arduino Serial library for communications, a future Raw Hid implementation is planned.

- ReflectaFunctions -- a remote function calling protocol that builds on top of ReflectaFrames.  Arduino functions are 'registered' for rpc by calling bind(Interface, Function Pointer).  ReflectaFunctions uses a stack-based approach to calling functions, meaning first you push(...) your parameters on your stack, then you invoke your functions, and your functions pop(...) their data off the stack.  ReflectaFunctions defines function ids '0' as PushArray to add parameter data to the stack and '1' as 'QueryInterface' to determine what Interfaces (method groups) this firmware implements. 

- ReflectaArduinoCore -- a binding of the Arduino core library functions such as pinMode, digitalRead, analogWrite to the 'ARDU1' interface.

- ReflectaHeartbeat -- an Arduino-side library for reading digital and analog pins very efficienctly and sending the results at a fixed frequency to the host PC.

## Getting Started ##

To install, download the contents of the repository as a zip file and decompress into your libraries folder, either where you have the Arduino IDE installed or in your ['sketchbook' folder](http://arduino.cc/it/Reference/Libraries) (on Windows this is My Documents -> arduino) in a subdirectory named 'libraries'.  When you are done you should have a file like My Documents\arduino\libraries\ReflectaFramesSerial\ReflectaFramesSerial.h.  Now you can restart the Arduino IDE and use Sketch -> Import Library.

## Examples ##

BasicReflecta -- simplest example of using Reflecta, opens a Reflecta listener at 19200 baud and exposes the Arduino Core functions to be called.  This sample is the 'Arduino side' of the conversation.