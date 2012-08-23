Reflecta is a node.js client for communicating with an Arduino via the Reflecta protocol.

### Getting Started
Install the module with: `npm install reflecta`

```javascript
var Reflecta = require('reflecta');
var reflecta = new Reflecta("/dev/ttyACM0");

reflecta.on('ready', function() {
  reflecta.once('response', function(response) {
    console.log(response);
  });

  reflecta.sendFrame([1]);
});
```

### Methods

```javascript
reflecta.sendFrame(buffer0, ..., bufferN);
```
Sends a frame of data to the Arduino comprised by concatenating the buffers.  Parameters will be auto-converted to a [NodeJS buffer](http://nodejs.org/api/buffer.html) so an array of octets (bytes) or a string is reasonable input.

Auto-generates a sequence number to help in detecting lost packets.
Auto-calculates a simple 8 bit CRC to help detect data corruption.  Auto-escapes data using SLIP escaping to 
turn a stream of data into deliniated frames.

Note:  Be sure that ready event has fired before calling sendFrame.

### Events

```javascript
reflecta.on('ready', function() ... );
```

The connection has been opened and reflecta is ready to be used to communicate with the Arduino.

```javascript
reflecta.on('error', function(error) ... );
```

An fatal error was detected in the protocol, such as a buffer overflow or underflow, function id conflict, or error with the communications port.

```javascript
reflecta.on('warning', function(warning) ... );
```

A non-fatal warning was detected in the protocol, anything from out of sequence (dropped frame) to bad CRC or
unexpected SLIP Escape (corrupted data)..

```javascript
reflecta.on('message', function(message) ... );
```

A string message was received.  Generally used for 'println debugging' from the Arduino.  'message' is a UTF8 string.

```javascript
reflecta.on('response', function(response) ... );
```

A response was received to a function executed on the Arduino by a frame sent from this client.

`response` contains properties

- `sequence` contains the sequence number of the frame this is a response to so you can correlate request/response pairs.
- `data` contains the byte[] data for the response.

```javascript
reflecta.on('frame', function(frame) ... );
```

A frame of data was received from the Arduino.  This event is only fired for frames that are not recognized as a known FrameType (e.g. Error, Warning, Message, Response, Heartbeat) by the buffer[0] value.

'frame' contains properties

- `sequence` contains the sequence number of this frame.
- `data` contains the byte[] data for the response.

```javascript
reflecta.on('heartbeat', function() ... );
```

A frame of heartbeat data was received from the Arduino.  Heartbeat is a scheduled delivery of data retrieved from the Arduino, such as the current reading of the digital io or analog io ports.

`heartbeat` contains properties

- `collectingLoops`: number of iterations through loop() while collecting heartbeat data
- `idleLoops`: number of iterations through loop() while waiting for heartbeat timer to expire
- `data`: heartbeat byte[]'s

```javascript
reflecta.on('close', function() ... );
```

The communications port was closed.  A light wrapper over node-serialport's close event.

```javascript
reflecta.on('end', function() ... );
```

The communications port was ended.  A light wrapper over node-serialport's end event.

```javascript
reflecta.on('open', function() ... );
```

The communications port is open.  A light wrapper over node-serialport's open event.

## Contributing
In lieu of a formal styleguide, take care to maintain the existing coding style. Add unit tests for any new or changed functionality. Lint and test your code using [grunt](https://github.com/cowboy/grunt).

## Release History
Still in early alpha state.  Subject to frequent breaking API changes at this time.

## License
Copyright (c) 2012 Jay Beavers  
Licensed under the BSD license.
