var fs = require('fs');
var events = require('events');
var util = require('util');
var path = require('path');

function Board(port, options) {
  
  if (!(this instanceof Board)) {
    return new Board(port, options);
  }

  var self = this;
  
  var serialPort = new (require("serialport").SerialPort)(port, options);

  // Query our interfaces and attach our interface libraries
  serialPort.once('open', function() {

    var timeoutId = setTimeout(function() {
      self.emit('error', 'Timeout awaiting response to QueryInterface');
    }, 1000);

    self.once('error', function(error) {
      clearTimeout(timeoutId);
    });

    self.queryInterface(function(interfaces) {
      clearTimeout(timeoutId);
      self.emit('ready');
    });    
  });
  
  // SLIP (http://www.ietf.org/rfc/rfc1055.txt) protocol special character definitions
  // Used to find end of frame when using a streaming communications protocol
  var EscapeCharacters = {
    END             : 0xC0,
    ESCAPE          : 0xDB,
    ESCAPED_END     : 0xDC,
    ESCAPED_ESCAPE  : 0xDD
  };
  
  var ReadState = {
    FrameStart      : 0, // Reading the frame sequence # at the start of the frame
    ReadingFrame    : 1, // Reading frame data bytes until an End is received
    FrameEnded      : 2,  // End received, processing the frame data
    FrameInvalid    : 3 // Waiting for the next End character to get a fresh frame state
  };
  
  var FrameTypes = {
    Reset       : 0x7A,
    Heartbeat   : 0x7B,
    Response    : 0x7C,
    Message     : 0x7D,
    Warning     : 0x7E,
    Error       : 0x7F
  };
  
  var ErrorCodes = {
    0x00 : 'OutOfSequence',
    0x01 : 'UnexpectedEscape',
    0x02 : 'CrcMismatch',
    0x03 : 'UnexpectedEnd',
    0x04 : 'BufferOverflow',
    0x05 : 'FrameTooSmall',
    0x06 : 'FunctionConflict',
    0x07 : 'FunctionNotFound',
    0x08 : 'ParameterMismatch',
    0x09 : 'StackOverflow',
    0x0A : 'StackUnderflow',
    0x0B : 'WireNotAvailable'
  };
    
  this.FunctionIds = {
    pushArray           : [ 0x00 ],
    queryInterface      : [ 0x01 ],
    sendResponse        : [ 0x02 ],
    sendResponseCount   : [ 0x03 ],
    reset               : [ 0x7A ]
  };

  var writeArray = []; // Space to compose an outgoing frame of data
  var writeArrayIndex = 0;
  
  var writeSequence = 0;
  var writeChecksum = 0;
  
  var writeEscaped = function(b) {
    switch(b) {

      case EscapeCharacters.END:
        writeArray[writeArrayIndex++] = EscapeCharacters.ESCAPE;
        writeArray[writeArrayIndex++] = EscapeCharacters.ESCAPED_END;
        break;
      case EscapeCharacters.ESCAPE:
        writeArray[writeArrayIndex++] = EscapeCharacters.ESCAPE;
        writeArray[writeArrayIndex++] = EscapeCharacters.ESCAPED_ESCAPE;
        break;
      default:
        writeArray[writeArrayIndex++] = b;
        break;
    }
    
    writeChecksum ^= b;
  };
    
  var readArray = [];
  var readArrayIndex = 0;
  
  var readState = ReadState.FrameStart;
  
  var readSequence = 0;
  var readChecksum = 0;
  
  var escaped = false;
  var readUnescaped = function() {
    
      var b = readArray[readArrayIndex++];
      
      if (escaped) {
        escaped = false;
        switch (b)
        {
          case EscapeCharacters.ESCAPED_END:
            return EscapeCharacters.END;
          case EscapeCharacters.ESCAPED_ESCAPE:
            return EscapeCharacters.ESCAPE;
          default:
            self.emit('warning', 'Local: frame corrupt, unexpected escape');
            readState = ReadState.FrameInvalid;
            return null;
        }
      }
      
      if (b === EscapeCharacters.ESCAPE) {
        escaped = true;
        return null;      
      } 
      
      if (b === EscapeCharacters.END) {
        switch (readState)
        {
          case ReadState.FrameInvalid:
            readState = ReadState.FrameStart;
            break;
          case ReadState.ReadingFrame:
            readState = ReadState.FrameEnded;
            break;
          default:
            self.emit('warning', 'Local: frame corrupt, unexpected end');
            readState = ReadState.FrameInvalid;
            break;
        }
      }
      
      return b;
  };
  
  var frameSequence = 0;
  var frameBuffer = []; // Buffer to hold a frame of data pulled out of the incoming communications stream
  var frameIndex = 0;
  
  var parseFrame = function(data) {
    
    readArray = data;
    readArrayIndex = 0;
          
    while (readArrayIndex < readArray.length)
    {
      var b = readUnescaped();
      
      // Either escape character or an error, either way ignore this data
      if (b == null) {
        continue;
      }
                                  
      switch (readState) {
        
        case ReadState.FrameInvalid:
          
          break;
          
        case ReadState.FrameStart:
          
          frameSequence = b;
          
          if (readSequence++ !== frameSequence) {
            readSequence = frameSequence + 1;
          }
          
          readChecksum = b; // Start off a new checksum
          frameBuffer = []; // Reinitialize the buffer since we send using frameBuffer.length
          frameIndex = 0; // Reset the buffer pointer to beginning

          readState = ReadState.ReadingFrame;
          
          break;
          
        case ReadState.ReadingFrame:
          
          frameBuffer[frameIndex++] = b;
          readChecksum ^= b;
          
          break;
          
        case ReadState.FrameEnded:
          // zero expected because when CRC of data is XORed with Checksum byte it should equal zero
          if (readChecksum !== 0) {
            
            self.emit('warning', 'Local: frame corrupt, crc mismatch', data, readChecksum);
            
          } else { 
            
            // Valid frame received, process based on frame type
            switch (frameBuffer[0]) {
              
              case FrameTypes.Error:
                
                self.emit('error', 'Remote: ' + frameBuffer[1] + ' : ' + ErrorCodes[frameBuffer[1]]);
                
                break;

              case FrameTypes.Warning:

                self.emit('warning', 'Remote: ' + frameBuffer[1] + ' : ' + ErrorCodes[frameBuffer[1]]);
                
                break;
    
              case FrameTypes.Message:
                
                var length = frameBuffer[1];
                self.emit('message', new Buffer(frameBuffer).toString('utf8', 2, length + 2));
    
                break;
                
              case FrameTypes.Response:
    
                var responseToSequence = frameBuffer[1];
                var frameLength = frameBuffer[2];
                var responseData = new Buffer(frameBuffer).slice(3, frameLength + 3);
                self.emit('response', { sequence: responseToSequence, data: responseData });
                
                break;
              
              case FrameTypes.Heartbeat:
                
                var collectingLoops = frameBuffer[1] + (frameBuffer[2] << 8);
                var idleLoops = frameBuffer[3] + (frameBuffer[4] << 8);
                self.emit('heartbeat', { collectingLoops: collectingLoops, idleLoops: idleLoops, data: new Buffer(frameBuffer).slice(5, frameBuffer.length - 1) });
                
                break;
              
              // FrameType unknown, use a generic frame received callback
              default:
                
                // TODO: consider expressing this as multiple params?
                self.emit('frame', { sequence: frameSequence, data: new Buffer(frameBuffer) });
                  
                break;
                
            }
          }
          
          readState = ReadState.FrameStart;
          break;
      }            
    }
  };
  
  serialPort.on('data', parseFrame);
  serialPort.on('open', function() { self.emit('open'); });
  serialPort.on('close', function() { self.emit('close'); });
  serialPort.on('end', function() { self.emit('end'); });
  serialPort.on('error', function(err) { self.emit('error', err); });
  
  events.EventEmitter.call(this);
  
  this.close = function(callback) {
    self.sendFrame(self.FunctionIds.reset);
    serialPort.close(callback);
  };
  
  this.sendFrame = function(frame, frame2, frame3, frame4) {
  
    if (!Buffer.isBuffer(frame)) {
      frame = new Buffer(frame);
    }
    
    // This smells like something better done with params :-)
    if (frame2) {

      if (!Buffer.isBuffer(frame2)) {
        frame2 = new Buffer(frame2);
      }

      frame = Buffer.concat( [frame, frame2] );
    }

    if (frame3) {

      if (!Buffer.isBuffer(frame3)) {
        frame3 = new Buffer(frame3);
      }

      frame = Buffer.concat( [frame, frame3] );
    }

    if (frame4) {
      if (!Buffer.isBuffer(frame4)) {
        frame4 = new Buffer(frame4);
      }

      frame = Buffer.concat( [frame, frame4] );
    }
    
    // Artificial 8-bit rollover
    if (writeSequence === 256) { 
      writeSequence = 0;
    }
    
    // Reinitialize the writeArray and checksum
    writeChecksum = 0;
    writeArray = [];
    writeArrayIndex = 0;
  
    writeEscaped(writeSequence);
    
    for (var index = 0; index < frame.length; index++) {
      writeEscaped(frame[index]);
    }
    
    writeEscaped(writeChecksum);
    
    writeArray[writeArrayIndex++] = EscapeCharacters.END;
    
    serialPort.write(writeArray);
    
    return writeSequence++;
  };
  
  // Push an 'n' count of 'parameter' data onto the stack where n == the next byte
  // in the frame after the pushArray function id, followed by the next n bytes of
  // data.
  // For example, a frame of [ 1 4 9 8 7 6 ] would mean:
  // [ 1 ]              pushArray
  //   [ 4 ]            count of parameter data
  //     [ 9 8 7 6 ]    parameter data to be pushed on the stack
  this.pushArray = function(array) {
    
    self.sendFrame(self.FunctionIds.pushArray, array);
    
  };
  
  // Query the interfaces (e.g. function groups) bound on the Arduino
  this.queryInterface = function(callback) {
    
    var callSequence = self.sendFrame(self.FunctionIds.queryInterface);
    
    var queryInterfaceHandler = function(response) {

      if (response.sequence === callSequence) {

        self.removeListener('response', queryInterfaceHandler);

        self.interfaces = {};

        for (var interfaceIndex = 0; interfaceIndex < response.data.length / 6; interfaceIndex++) {

          var interfaceOffset = response.data[interfaceIndex * 6];
          var interfaceId = response.data.slice(interfaceIndex * 6 + 1, interfaceIndex * 6 + 6).toString();
          var interfaceModule = 'reflecta_' + interfaceId;

          try {
            // Try and find the interface module locally
            self[interfaceId] = require(interfaceModule)(self, interfaceOffset);
            self.interfaces[interfaceId] = self[interfaceId];
          }
          catch (error) {
            self.emit('message', 'QueryInterface: local interface definition not found for ' + interfaceId);

            // If not found locally, search for it in the NPM registry
            var searchRegistry = (function(interfaceId) {
              var interfaceModule = 'reflecta_' + interfaceId;
              console.log('searching for ' + interfaceId);
              var npm = require('npm');
              npm.load(function(error, npm) {
                npm.install(interfaceModule, function(error) {
                  if (error) {
                    self.emit('warning', 'QueryInterface: npm registry interface definition not found for ' + interfaceId + ', ' + error);
                  } else {
                    self[interfaceId] = require(interfaceModule)(self, interfaceOffset);
                    self.interfaces[interfaceId] = self[interfaceId];
                  }
                });
              });                
            }(interfaceId));
          }
        }

        callback(self.interfaces);
      }
    };

    self.on('response', queryInterfaceHandler);
  };
  
  // Request a response with n bytes from the stack, where n == the first byte on the stack
  this.sendResponseCount = function(count, callback) {
    
    var callSequence = self.sendFrame( [self.FunctionIds.pushArray, 1, count, self.FunctionIds.sendResponseCount] );
    
    // TODO: Tighten logic not to assume ours must be the next response
    self.once('response', function(response) {
      if (response.sequence === callSequence) {
        callback(response.data);
      }
      
    });
  };
  
  // Request a response with 1 byte from the stack
  this.sendResponse = function(callback) {
    
    var callSequence = self.sendFrame(self.FunctionIds.sendResponse);
    
    // TODO: Tighten logic not to assume ours must be the next response
    self.once('response', function(response) {
      if (response.sequence === callSequence) {
        callback(response.data);
      }

    });
  };
}

util.inherits(Board, events.EventEmitter);

// Try and programmatically detect an Arduino by inspecting the available serial port devices
function detect(options, callback) {

  if (callback === undefined) {
    callback = options;
    options = undefined;
  }

  require("serialport").list(function (error, results) {
    if (error) {
      callback(error);
      return;
    }

    var port;
    for (var i = 0; i < results.length; i++) {
      var item = results[i];

      // Under Windows this catches any Arduino that is loaded using the Teensyduino INF.
      // Hardcoded to detect a Teensy INF device for now.  Teensy INF works with Leonardo too.
      if (item.manufacturer && item.manufacturer.indexOf('PJRC') !== -1) {
        port = results[i].comName;
        break;
      }

      // Under Ubuntu 12.04 this catches a Leonardo.
      if (item.pnpId && item.pnpId.indexOf('Arduino') !== -1) {
        port = results[i].comName;
        break;          
      }

      // Under Ubuntu 12.04 this catches a Teensy.
      if (item.pnpId && item.pnpId.indexOf('Teensy') !== -1) {
        port = results[i].comName;
        break;          
      }
    }

    if (port) {

      var board = new Board(port, options);

      board.once('error', function(error) {
        board.removeAllListeners('ready');
        // TODO: rather than call the callback with error, simply remove this port 'from consideration'
        callback(error, null, [ port ]);
      });

      board.once('ready', function() {
        // TODO: iterate through all ports, don't stop with first found, it may not be
        // responding to Reflecta queryInterface or it may already be in use
        callback(null, [ board ], [ port ]);
      });

    } else {
      callback('Arduino not found on available serial ports', null, results.map(function(result) { return result.comName; }));
    }
  });
}

module.exports = { Board: Board, detect: detect };