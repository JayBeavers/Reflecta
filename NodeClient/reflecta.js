var fs = require('fs');
var events = require('events');
var util = require('util');

function Reflecta(path, options, callback) {
  
  if (!(this instanceof Reflecta)) {
    return new Reflecta(path, options, callback);
  }

  var self = this;

  if (!callback || typeof callback != "function") {
    callback = options;   // callback must be the second parameter
    options = undefined;  // no option passed
  }
  
  var serialPort = new (require("serialport").SerialPort)(path, options);

  // Query our interfaces and attach our interface libraries
  serialPort.once('open', function() {
    self.queryInterface(function(interfaces) {
      if (callback) {
        callback();
      }
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
    WaitingForEnd   : 0, // Waiting for the next End character to get a fresh frame state
    ReadingSequence : 1, // Reading the frame sequence # at the start of the frame
    ReadingFrame    : 2, // Reading frame data bytes until an End is received
    ProcessingFrame : 3  // End received, processing the frame data
  };
  
  var FrameTypes = {
    Heartbeat   : 0x7B,
    Response    : 0x7C,
    Message     : 0x7D,
    Warning     : 0x7E,
    Error       : 0x7F
  };
  
  var ErrorCodes = {
    0x00 : 'FRAMES_WARNING_OUT_OF_SEQUENCE',
    0x01 : 'FRAMES_WARNING_UNEXPECTED_ESCAPE',
    0x02 : 'FRAMES_WARNING_CRC_MISMATCH',
    0x03 : 'FRAMES_WARNING_UNEXPECTED_END',
    0x04 : 'FRAMES_ERROR_BUFFER_OVERFLOW',
    0x05 : 'FUNCTIONS_ERROR_FRAME_TOO_SMALL',
    0x06 : 'FUNCTIONS_ERROR_FUNCTION_CONFLICT',
    0x07 : 'FUNCTIONS_ERROR_FUNCTION_NOT_FOUND',
    0x08 : 'FUNCTIONS_ERROR_PARAMETER_MISMATCH',
    0x09 : 'FUNCTIONS_ERROR_STACK_OVERFLOW',
    0x0A : 'FUNCTIONS_ERROR_STACK_UNDERFLOW'
  };
  
  this.FunctionIds = {
    pushArray           : [ 0x00 ],
    queryInterface      : [ 0x01 ],
    sendResponse        : [ 0x02 ],
    sendResponseCount   : [ 0x03 ]
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
            self.emit('warning', 'Local: frame corrupt, unexpected escape');
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
            self.emit('warning', 'Local: frame corrupt, unexpected end');
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
                var length = frameBuffer[2];
                var message = new Buffer(frameBuffer).slice(3, length + 3);
                self.emit('response', { sequence: responseToSequence, message: message });
                
                break;
              
              case FrameTypes.Heartbeat:
                
                var collectingLoops = frameBuffer[1] + (frameBuffer[2] << 8);
                var idleLoops = frameBuffer[3] + (frameBuffer[4] << 8);
                self.emit('heartbeat', { collectingLoops: collectingLoops, idleLoops: idleLoops, data: new Buffer(frameBuffer).slice(5, frameBuffer.length - 1) });
                
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
    
    var callSequence = self.sendFrame(self.FunctionIds.queryInterface);
    
    // TODO: Tighten logic not to assume ours must be the next response
    self.once('response', function(response) {

      if (response.sequence == callSequence) {

          self.interfaces = {};

          for (var interfaceIndex = 0; interfaceIndex < response.message.length / 6; interfaceIndex++) {
            var interfaceOffset = response.message[interfaceIndex * 6];
            var interfaceId = response.message.slice(interfaceIndex * 6 + 1, interfaceIndex * 6 + 6).toString();
            var interfaceFilePath = './interfaces/' + interfaceId;

            if (fs.exists(interfaceFilePath)) {
              self[interfaceId] = require(interfaceFilePath)(self, interfaceOffset);
              self.interfaces[interfaceId] = self[interfaceId];
            }
          }

          callback(self.interfaces);
        }
    });
  };
  
  // Request a response with n bytes from the stack, where n == the first byte on the stack
  this.sendResponseCount = function(count, callback) {
    
    var callSequence = self.sendFrame(self.FunctionIds.sendResponseCount, count);
    
    // TODO: Tighten logic not to assume ours must be the next response
    self.once('response', function(response) {
      if (response.sequence == callSequence) {
        callback(null, response.message);
      }
      
    });
  };
  
  // Request a response with 1 byte from the stack
  this.sendResponse = function(callback) {
    
    var callSequence = self.sendFrame(FunctionIds.sendResponse);
    
    // TODO: Tighten logic not to assume ours must be the next response
    self.once('response', function(response) {
      if (response.sequence == callSequence) {
        callback(null, response.message);
      }

    });
  };
};

util.inherits(Reflecta, events.EventEmitter);

module.exports = Reflecta;