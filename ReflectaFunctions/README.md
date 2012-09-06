# ReflectaFunctions #

> _Stability: Low_

Bind a function to an interfaceId so it can be called by Reflecta Functions.  Function receives the Sequence number of the calling frame (to be used when sending a response) and the byte* parameters.
>  byte bind(String interfaceId, void (*function)(byte callerSequence, byte parameterLength, byte* parameters));

Send a response to a function call.  Match the sequence of your caller to allow the host computer to correlate the request & response.
> void sendResponse(byte callerSequence, byte parameterLength, byte* parameters);

reflectaFunctions setup() to be called in the Arduino setup() method.
> void setup();

---

It's assumed that a higher level Reflecta Functions client library will hide this detail from the caller, instead exposing a simple function API that uses QueryInterface and sendFrame behind the scenes to make function calls appear local.

For an example of this, see the [NodeJS client library](https://github.com/JayBeavers/Reflecta/tree/master/NodeClient).

## Release History

- 0.3.x: Initial release

## Futures

- May renumber FunctionIds, don't depend on specific values here yet
- Add push32 & pop32 to support microSeconds parameters