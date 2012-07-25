var events = require('events');
var util = require('util');
var serialPort = new (require("serialport").SerialPort)('/dev/ttyACM0');

function Reflecta() {
  
  if (false === (this instanceof Reflecta)) {
    return new Reflecta();
  }

  var self = this;
  
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
    Response : 0x7D,
    Message  : 0x7E,
    Error    : 0x7F
  };
  
  var ErrorCodes = {
    0x00 : 'FRAMES_WARNING_OUT_OF_SEQUENCE',
    0x01 : 'FRAMES_ERROR_UNEXPECTED_ESCAPE',
    0x02 : 'FRAMES_ERROR_CRC_MISMATCH',
    0x03 : 'FRAMES_ERROR_UNEXPECTED_END',
    0x04 : 'FRAMES_ERROR_BUFFER_OVERFLOW'
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
  
  var sendFrame = function(frame) {
  
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
  
  var readArray = [];
  var readArrayIndex = 0;
  
  var escaped = false;
  var readState = ReadState.ReadingSequence;
  
  var readSequence = 0;
  var readChecksum = 0;
  
  var escaped = false;
  var readUnescaped = function() {
      var b = readArray[readArrayIndex++];
      
      if (escaped) {
        switch (b)
        {
          case EscapeCharacters.ESCAPED_END:
            return EscapeCharacters.END;
          case EscapeCharacters.ESCAPED_ESCAPE:
            return EscapeCharacters.ESCAPE;
          default:
            console.log('unexpected escape');
            self.emit('error', 'Local: frame corrupt, unexpected escape');
            readState = ReadState.WaitingForEnd;
            return null;
        }
      }
      
      if (b == EscapeCharacters.ESCAPE) {
        escaped = true;
        return null;      
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
        
        if (b == EscapeCharacters.END) { // end of frame detected, process the data accumulated in the frameBuffer
          
          if (readState == ReadState.WaitingForEnd) {
            // throw away the previous data and start fresh
            readState = ReadState.ReadingSequence;
            continue;
          }
          
          // zero expected because when CRC of data is XORed with Checksum byte it should equal zero
          if (readChecksum != 0) {
            console.log('Node crc mismatch');
            self.emit('error', 'Local: frame corrupt, crc mismatch');
            
          } else { 
            
            // Valid frame received, process based on frame type
            switch (frameBuffer[0]) {
              
              case FrameTypes.Error:
                
                console.log("Node error: " + ErrorCodes[frameBuffer[1]]);
                self.emit('error', 'Remote: ' + ErrorCodes[frameBuffer[1]]);
                
                break;
    
              case FrameTypes.Message:
                
                var length = frameBuffer[1];
                console.log("Node Message: " + new Buffer(frameBuffer).toString('utf8', 2, length + 2));
                self.emit('message', new Buffer(frameBuffer).toString('utf8', 2, length + 2));
    
                break;
                
              case FrameTypes.Response:
    
                var responseToSequence = frameBuffer[1];
                var length = frameBuffer[2];
                // Broken, assumes the response is a text message!!!
                var message = new Buffer(frameBuffer).toString('utf8', 3, length + 3);
                console.log("Node Response to " + responseToSequence.toString(16) + " : " + message);
                self.emit('response', { sequence: responseToSequence, message: message });
                
                break;
              
              // FrameType unknown, use a generic frame received callback
              default:
                
                // TODO: consider expressing this as multiple params?
                self.emit('frame', { sequence: frameSequence, data: frameBuffer });
                  
                break;
                
            }
          }
          
          readState = ReadState.ReadingSequence;
          continue;
        }
        
        switch (readState) {
          
          case ReadState.WaitingForEnd:
            console.log('w ' + b.toString(16));
            
            break;
            
          case ReadState.ReadingSequence:
            
            frameSequence = b;
            
            if (readSequence++ != frameSequence) {
              console.log("Node Expected " + (readSequence - 1).toString(16) + " received " + frameSequence.toString(16) );
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
      }
    }
  }
  
  serialPort.on('data', parseFrame);
  serialPort.on('open', function() { console.log('open');  self.emit('portOpen'); });
  serialPort.on('close', function() { console.log('close'); self.emit('portClose'); });
  serialPort.on('end', function() { console.log('end'); self.emit('portEnd'); });
  serialPort.on('error', function(err) { console.log('error '  + err); self.emit('portError', err); });
  
  events.EventEmitter.call(this);
  
  this.sendFrame = sendFrame;
}

util.inherits(Reflecta, events.EventEmitter);

module.exports = Reflecta;
