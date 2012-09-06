Reflecta is a node.js client for communicating with an Arduino via the Reflecta protocol.

> _Stability: Low_

### Getting Started
Install the module with: `npm install reflecta`

```javascript
var reflecta = require('reflecta');
var board = new reflecta.Board("/dev/ttyACM0");

board.on('ready', function() {
  board.once('response', function(response) {
    console.log(response);
  });

  board.sendFrame([1]);
});
```

### Detecting Arduino Boards

```javascript
var reflecta = require('reflecta');
reflecta.detect(function(error, boards, ports) {

  // Current implementation only returns the first board found
  var board = boards[0];

  // board is not returned until 'ready' has already fired
  board.once('response', function(response) {
    console.log(response);
  });

  board.sendFrame([1]);

});
```

### Board Interfaces

On startup Reflecta probes the Arduino to see what libraries are installed using QueryInterface.  If it finds interfaces, it will automatically load a matching set of functions onto the Reflecta object using node's require or an npm install.  A few well known interfaces that are supported are:

__[ARDU1](https://github.com/JayBeavers/Reflecta/blob/master/NodeClient/node_modules/reflecta_ARDU1.js)__

The ARDU1 interface exposes the Arduino Digital & Analog I/O Functions and Wire and Servo libraries.

__[MOTO1](https://github.com/JayBeavers/Reflecta/blob/master/NodeClient/node_modules/reflecta_MOTO1.js)__

The MOTO1 interface exposes the SparkFun Monster Moto shield with functions like Drive, Brake, and ReadCurrent.

__[HART1](https://github.com/JayBeavers/Reflecta/blob/master/NodeClient/node_modules/reflecta_HART1.js)__

The HART1 interface exposes the settings functions for the Reflecta Heartbeat library such as setFrameRate.

__[RBOT1](https://github.com/JayBeavers/Reflecta/blob/master/NodeClient/node_modules/reflecta_RBOT1.js)__

The RBOT1 interface exposes the commands for [RocketBot](https://github.com/JayBeavers/RocketBot) to launch pneumatic straw rockets and control blinky lights such as Animation, Compressor, Valve, and Fire.

An example of how to use interfaces:
```javascript
var reflecta = require('reflecta');
var board = new reflecta.Board("/dev/ttyACM0");

board.on('ready', function() {

  board.ARDU1.digitalWrite(7, 1); // Set digital pin 7 to on
  board.MOTO1.drive(50, 50); // Set left and right wheel powers to 50 out of 255
  board.RBOT1.Fire(); // Fire a straw rocket

});
```

### Board Methods

```javascript
board.sendFrame(buffer0, ..., bufferN);
```
Sends a frame of data to the Arduino comprised by concatenating the buffers.  Parameters will be auto-converted to a [NodeJS buffer](http://nodejs.org/api/buffer.html) so an array of octets (bytes) or a string is reasonable input.

Auto-generates a sequence number to help in detecting lost packets.
Auto-calculates a simple 8 bit CRC to help detect data corruption.  Auto-escapes data using SLIP escaping to 
turn a stream of data into deliniated frames.

Note:  Be sure that ready event has fired before calling sendFrame.

### Board Events

```javascript
board.on('ready', function() ... );
```

The connection has been opened and reflecta is ready to be used to communicate with the Arduino.

```javascript
board.on('error', function(error) ... );
```

An fatal error was detected in the protocol, such as a buffer overflow or underflow, function id conflict, or error with the communications port.

```javascript
board.on('warning', function(warning) ... );
```

A non-fatal warning was detected in the protocol, anything from out of sequence (dropped frame) to bad CRC or
unexpected SLIP Escape (corrupted data)..

```javascript
board.on('message', function(message) ... );
```

A string message was received.  Generally used for 'println debugging' from the Arduino.  'message' is a UTF8 string.

```javascript
board.on('response', function(response) ... );
```

A response was received to a function executed on the Arduino by a frame sent from this client.

`response` contains properties

- `sequence` contains the sequence number of the frame this is a response to so you can correlate request/response pairs.
- `data` contains the byte[] data for the response.

```javascript
board.on('frame', function(frame) ... );
```

A frame of data was received from the Arduino.  This event is only fired for frames that are not recognized as a known FrameType (e.g. Error, Warning, Message, Response, Heartbeat) by the buffer[0] value.

`frame` contains properties

- `sequence` contains the sequence number of this frame.
- `data` contains the byte[] data for the response.

```javascript
board.on('heartbeat', function() ... );
```

A frame of heartbeat data was received from the Arduino.  Heartbeat is a scheduled delivery of data retrieved from the Arduino, such as the current reading of the digital io or analog io ports.

`heartbeat` contains properties

- `collectingLoops`: number of iterations through loop() while collecting heartbeat data
- `idleLoops`: number of iterations through loop() while waiting for heartbeat timer to expire
- `data`: heartbeat byte[]'s

```javascript
board.on('close', function() ... );
```

The communications port was closed.  A light wrapper over node-serialport's close event.

```javascript
board.on('end', function() ... );
```

The communications port was ended.  A light wrapper over node-serialport's end event.

```javascript
board.on('open', function() ... );
```

The communications port is open.  A light wrapper over node-serialport's open event.

## Contributing
In lieu of a formal styleguide, take care to maintain the existing coding style. Add unit tests for any new or changed functionality. Lint and test your code using [grunt](https://github.com/cowboy/grunt).

## Release History

- 0.3.x: Still in early state.  Subject to frequent breaking API changes at this time.
- 0.3.3: Added reflecta.detect(...), renamed ctor from Reflecta to Board to match firmata/johnny-five

## Futures

See [this Trello Board](https://trello.com/board/reflecta/4fe0b182caf51043640db94b) for planned work.

## License
Copyright (c) 2012 Jay Beavers  
Licensed under the BSD license.
