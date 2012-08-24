# ReflectaArduinoCore #

Reflecta Arduino Core binds the functions pinMode, digitalRead, digitalWrite, analogRead, and analogWrite to the ARDU1 interface.  This is the Arduino library that enables the 'send' side of the Reflecta conversation.

> _Stability: Beta_ -- no plans to change existing APIs but may make additions, taking community feedback, documenting breaking changes

## Calling ReflectaArduinoCore from NodeJS

Documentation on using the NodeJS client can be found [here](https://github.com/JayBeavers/Reflecta/blob/master/NodeClient/node_modules/ARDU1.md).

## To Be Documented -- Calling ReflectaArduinoCore 'raw' just using ReflectaFunctions

## Release History

- 0.3.x: Beta release ReflectaArduinoCore with analog & digital io, wire, and servo.

## Futures

- Add Advanced I/O functions (tone, pulseIn, etc.)
- Consider including other Arduino standard libraries such as Stepper, EEPROM, SD, SPI, SoftwareSerial, etc.