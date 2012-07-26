Overview
========

Reflecta is a node.js client for communicating with an Arduino via the Reflecta protocol.

### Sample Usage

> var Reflecta = require('./reflecta');

> var reflecta = new Reflecta();

> reflecta.on('response', function(response) { console.log(response.sequence + ' : ' + response.message); });

> reflecta.on('portOpen', function() { reflecta.sendFrame([1]); });

### Methods

#### reflecta.sendFrame( buffer );

Sends a frame of data to the Arduino.  Auto-generates a sequence number to help in detecting lost packets.
Auto-calculates a simple 8 bit CRC to help detect data corruption.  Auto-escapes data using SLIP escaping to 
turn a stream of data into deliniated frames.

Note:  Be sure that portOpen event has fired before calling sendFrame.  Will be sure to clean this up and
simplify in future releases

### Events

#### reflecta.on('error', function(err) ... );

An error was detected in the protocol, anything from out of sequence (dropped frame) to bad CRC or
unexpected SLIP Escape (corrupted data).

#### reflecta.on('message', function(message) ... );

A string message was received.  Generally used for 'println debugging' from the Arduino.

#### reflecta.on('response', function(response) ... );

A response was received to a function executed on the Arduino by a frame sent from this client.

#### reflecta.on('frame', function(frame) ... );

A frame of data was received from the Arduino.

#### reflecta.on('portClose', function() ... );

The communications port was closed.  A light wrapper over node-serialport's close event.

#### reflecta.on('portEnd', function() ... );

The communications port was ended.  A light wrapper over node-serialport's end event.

#### reflecta.on('portError', function(err) ... );

The communications port had an error.  A light wrapper over node-serialport's error event.

#### reflecta.on('portOpen', function() ... );

The communications port is open.  A light wrapper over node-serialport's open event.