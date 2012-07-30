var events = require('events');
var util = require('util');

function Reflecta(path, options, callback) {
  
  if (false === (this instanceof Reflecta)) {
    return new Reflecta();
  }

  var self = this;

  if (!callback || typeof callback != "function") {
    callback = options;   // callback must be the second parameter
    options = undefined;  // no option passed
  }
  
  var serialPort = new (require("serialport").SerialPort)(path, options);
  if (callback) {
    serialPort.once('open', callback);
  }
  
  // SLIP (http://www.ietf.org/rfc/rfc1055.txt) protocol special character definitions
  // Used to find end of frame when using a streaming communications protocol
  var EscapeCharacters = {
    END             : 0xC0,
    ESCAPE          : 0xDB,
    ESCAPED_END     : 0xDC,
    ESCAPED_ESCAPE  : 0xDD
  };
  
  var ReadState = {
    WaitingForEnd   : 0, // Waiting for the next End character to get a fresh frame state
    ReadingSequence : 1, // Reading the frame sequence # at the start of the frame
    ReadingFrame    : 2, // Reading frame data bytes until an End is received
    ProcessingFrame : 3  // End received, processing the frame data
  };
  
  var FrameTypes = {
    Heartbeat   : 0x7A,
    Response    : 0x7D,
    Message     : 0x7E,
    Error       : 0x7F
  };
  
  var ErrorCodes = {
    0x00 : 'FRAMES_WARNING_OUT_OF_SEQUENCE',
    0x01 : 'FRAMES_ERROR_UNEXPECTED_ESCAPE',
    0x02 : 'FRAMES_ERROR_CRC_MISMATCH',
    0x03 : 'FRAMES_ERROR_UNEXPECTED_END',
    0x04 : 'FRAMES_ERROR_BUFFER_OVERFLOW'
  };
  
  var FunctionIds = {
    pushArray           : [ 0x00 ],
    queryInterface      : [ 0x01 ],
    sendResponseCount   : [ 0x7B ],
    sendResponse        : [ 0x7C ]
  };
  
  var writeArray = []; // Space to compose an outgoing frame of data
  var writeArrayIndex = 0;
  
  var writeSequence = 0;
  var writeChecksum = 0;
  
  var writeEscaped = function(b)
    {
      switch(b)
      {
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
    }
    
  var readArray = [];
  var readArrayIndex = 0;
  
  var readState = ReadState.ReadingSequence;
  
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
            self.emit('error', 'Local: frame corrupt, unexpected escape');
            readState = ReadState.WaitingForEnd;
            return null;
        }
      }
      
      if (b == EscapeCharacters.ESCAPE) {
        escaped = true;
        return null;      
      } 
      
      if (b == EscapeCharacters.END) {
        switch (readState)
        {
          case ReadState.WaitingForEnd:
            readState = ReadState.ReadingSequence;
            break;
          case ReadState.ReadingFrame:
            readState = ReadState.ProcessingFrame;
            break;
          default:
            self.emit('error', 'Local: frame corrupt, unexpected end');
            readState = ReadState.WaitingForEnd;
            break;
        }
      }
      
      return b;
  }
  
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
        
        case ReadState.WaitingForEnd:
          
          break;
          
        case ReadState.ReadingSequence:
          
          frameSequence = b;
          
          if (readSequence++ != frameSequence) {
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
          
        case ReadState.ProcessingFrame:
          // zero expected because when CRC of data is XORed with Checksum byte it should equal zero
          if (readChecksum != 0) {
            
            self.emit('error', 'Local: frame corrupt, crc mismatch', data, readChecksum);
            
          } else { 
            
            // Valid frame received, process based on frame type
            switch (frameBuffer[0]) {
              
              case FrameTypes.Error:
                
                self.emit('error', 'Remote: ' + frameBuffer[1] + ' : ' + ErrorCodes[frameBuffer[1]]);
                
                break;
    
              case FrameTypes.Message:
                
                var length = frameBuffer[1];
                self.emit('message', new Buffer(frameBuffer).toString('utf8', 2, length + 2));
    
                break;
                
              case FrameTypes.Response:
    
                var responseToSequence = frameBuffer[1];
                var length = frameBuffer[2];
                // Broken, assumes the response is a text message!!!
                var message = new Buffer(frameBuffer).toString('utf8', 3, length + 3);
                self.emit('response', { sequence: responseToSequence, message: message });
                
                break;
              
              case FrameTypes.Heartbeat:
    
                var freeCycles = frameBuffer[1] + (frameBuffer[0] << 8);
                self.emit('heartbeat', { freeCycles: freeCycles, data: new Buffer(frameBuffer).slice(3) });
                
                break;
              
              // FrameType unknown, use a generic frame received callback
              default:
                
                // TODO: consider expressing this as multiple params?
                self.emit('frame', { sequence: frameSequence, data: frameBuffer, frame: data });
                  
                break;
                
            }
          }
          
          readState = ReadState.ReadingSequence;
          break;
      }            
    }
  }
  
  serialPort.on('data', parseFrame);
  serialPort.on('open', function() { self.emit('portOpen'); });
  serialPort.on('close', function() { self.emit('portClose'); });
  serialPort.on('end', function() { self.emit('portEnd'); });
  serialPort.on('error', function(err) { self.emit('portError', err); });
  
  events.EventEmitter.call(this);
  
  this.close = function(callback) { serialPort.close(callback); };
  
  this.sendFrame = function(frame, frame2, frame3, frame4) {
  
    if (!Buffer.isBuffer(frame)) frame = new Buffer(frame);
    
    // This smells like something better done with params :-)
    if (frame2) {
      if (!Buffer.isBuffer(frame2)) frame2 = new Buffer(frame2);
      frame = Buffer.concat( [frame, frame2] );
    }

    if (frame3) {
      if (!Buffer.isBuffer(frame3)) frame3 = new Buffer(frame3);
      frame = Buffer.concat( [frame, frame3] );
    }

    if (frame4) {
      if (!Buffer.isBuffer(frame4)) frame4 = new Buffer(frame4);
      frame = Buffer.concat( [frame, frame4] );
    }
    
    // Artificial 8-bit rollover
    if (writeSequence == 256) writeSequence = 0;
    
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
    
    sendFrame(FunctionIds.pushArray, array);
    
  };
  
  // Query the interfaces (e.g. function groups) bound on the Arduino
  this.queryInterface = function(callback) {
    
    var callSequence = sendFrame(FunctionIds.queryInterface);
    
    self.once('response', function(response) {
      if (response.sequence == callSequence) {
        callback(null, response.message);
      }
      
    });
  };
  
  // Request a response with n bytes from the stack, where n == the first byte on the stack
  this.sendResponseCount = function(count, callback) {
    
    var callSequence = sendFrame(FunctionIds.sendResponseCount, count);
    
    self.once('response', function(response) {
      if (response.sequence == callSequence) {
        callback(null, response.message);
      }
      
    });
  };
  
  // Request a response with 1 byte from the stack
  this.sendResponse = function(callback) {
    
    var callSequence = sendFrame(FunctionIds.sendResponse);
        
    self.once('response', function(response) {
      if (response.sequence == callSequence) {
        callback(null, response.message);
      }

    });
  };
  
  var interfaceStart = 2;
  this.ardu1 = {
    
    gpio : {
      pinMode : function(pin, mode) { console.log(interfaceStart); sendFrame(interfaceStart); },

      digitalRead : function(pin, callback) {
        // This smells like we need a 'sendFrameWithResponseCallback'
        self.sendFrame( [FunctionIds.pushArray, 1, pin, interfaceStart + 1] );
        self.sendResponse(callback);
      },

      digitalWrite : function(pin, value) {
        self.sendFrame( [FunctionIds.pushArray, 2, pin, value, interfaceStart + 2] );
      },

      analogRead : function() { sendFrame(interfaceStart + 3); },
      analogWrite : function() { sendFrame(interfaceStart + 4); },
      pulseIn : function() { sendFrame(interfaceStart + 17); }
    },
    
    wire : {
      beginMaster : function() { sendFrame(interfaceStart + 5); },
      requestFrom : function() { sendFrame(interfaceStart + 6); },
      requestFromStart : function() { sendFrame(interfaceStart + 7); },
      available : function() { sendFrame(interfaceStart + 8); },
      read : function() { sendFrame(interfaceStart + 9); },
      beginTransmission : function() { sendFrame(interfaceStart + 10); },
      write : function() { sendFrame(interfaceStart + 11); },
      endTransmission : function() { sendFrame(interfaceStart + 12); }
    },
    
    servo : {
      attach : function() { sendFrame(interfaceStart + 13); },
      detach : function() { sendFrame(interfaceStart + 14); },
      write : function() { sendFrame(interfaceStart + 15); },
      writeMicroseconds : function() { sendFrame(interfaceStart + 16); }
    }
  };
  
}

util.inherits(Reflecta, events.EventEmitter);

module.exports = Reflecta;
