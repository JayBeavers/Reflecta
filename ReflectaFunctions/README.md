# ReflectaFunctions #

## Warning: obsolete ##

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
