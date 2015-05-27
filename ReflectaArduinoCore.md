# ReflectaArduinoCore #

Reflecta Arduino Core binds the functions pinMode, digitalRead, digitalWrite, analogRead, and analogWrite to the ARDU1 interface.  This is the Arduino library that enables the 'send' side of the Reflecta conversation.


## Calling ReflectaArduinoCore from NodeJS

The NodeJS client can be found [here](https://github.com/jaybeavers/node-reflecta/blob/master/node_modules/reflecta_ARDU1.js).

## Calling ReflectaArduinoCore 'raw' just using ReflectaFunctions

ReflectaFunctions implements a [Forth-like](http://en.wikipedia.org/wiki/Forth_%28programming_language%29) stack-based protocol.  To call a function like set pin 1 to input, in the Arduino sketch this would look like:

    pinMode(1, 0);

Using a Forth-like language, this would look like:

    push 1
    push 0
    execute pinMode

To read data, in an Arduino sketch this would look like:

    digitalRead(1);

and Forth-like:

    push 1
    execute digitalRead
    pop value

ReflectaFrames has an API to send and receive data over the wire:

 * sendFrame: push a byte[] buffer over the wire and into the Forth-like execution engine on the Arduino
 * responseReceived: received a response back from the Forth-like execution engine

ReflectaFunctions then adds a few primitive functions you can call via sendFrame

 * pushArray (0x00), add the next 'n' bytes to the stack
 * sendResponse (0x02), pop a byte off the stack and return it into responseReceived
 * sendResponseCount (0x03), pop the next 'n' bytes off the stack
 * queryInterface (0x01), ask the Arduino's execution engine to return a list of interfaces implemented in the sketch.  We'll cover this later.

So using the example above of:

    pinMode(1, 0);
    digitalRead(1);

This would look like:

    sendFrame [ pushArray, 2, 1, 0, pinMode ]
    sendFrame [ pushArray, 1, 1, digitalRead, sendResponse ]
    responseReceived [ pin1DigitalValue ]

In the above example, pushArray (0x00) and sendReponse (0x02) are constants.  What are the values of pinMode and digitalRead?  This is where queryInterface (0x01) comes in.  When you send queryInterface:

    sendFrame [ queryInterface ]

you will receive a matching response containing six byte data sets.  The first byte contains the 'offset', e.g. the address of the first function in the interface.  The next five bytes contains a UTF8 string.  For example,

    responseReceived [ 4, 'a', 'r', 'd', 'u', '1' ]

You then need to know the order of functions in the interface to know their offset within the interface.  In the case of 'ardu1', pinMode has offset 0 and digitalRead has offset 1.

to put it all together and use straight data rather than consts:

    // queryInterface
    sendFrame [ 1 ]
    responseReceived [ 4, 'a', 'r', 'd', 'u', '1' ]

    // pinMode(1, 0);
    //   pushFrame (0x00) of 2 bytes [ 1 0 ]
    //   execute pinMode at address 4 + 0
    sendFrame [ 0, 2, 1, 0, 4 ]

    // digitalRead(1);
    //   pushFrame (0x00) of 1 byte [ 1 ]
    //   execute digitalRead at address 4 + 1
    //   sendResponse (0x02)
    sendFrame [ 0, 1, 1, 5, 2 ]

    responseReceived [ 1 ]

You can also combine frames, so

    pinMode(1, 0);
    digitalRead(1);

Could have been written instead as:

    sendFrame [ 0, 2, 1, 0, 4, 0, 1, 1, 5, 2 ]
    responseReceived [ 1 ]


## Release History

- 0.3.x: Analog & digital io, wire, and servo.

## Futures

- Add Advanced I/O functions (tone, pulseIn, etc.)
- Consider including other Arduino standard libraries such as Stepper, EEPROM, SD, SPI, SoftwareSerial, etc.
