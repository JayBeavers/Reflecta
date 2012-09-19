# ReflectaHeartbeat #

Reflecta Heartbeat is a library that simplifies the common task of collecting data from a collection of sources on the Arduino (digital pins, analog pins, I2C devices, etc.), packaging all the results together into a 'heartbeat' frame, and then sending the results over USB at a set frame rate.

The design is basically a 'smart loop()' -- Heartbeat gathers the output of an array of async functions.  Each time loop() is called, the function array is checked to see if any functions have not yet completed for this heartbeat.  If not, the function is invoked and is expected to not block but return 'true' if done or 'false' if not.  Once all functions are done, the heartbeat data is sent.

## Performance Counters ##

Reflecta Heartbeat calculates two values while doing this: collectingLoops and idleLoops.  collectingLoops is the number of times heartbeat loop() was called and there was at least one heartbeat function not yet done.  idleLoops is the number of times heartbeat loop() was called and all the heartbeat functions were done but the framesPerSecond timer had not expired.  These two values let you measure 'cpu idle' on the Arduino -- if your collectionLoops is 'high' and your idleLoops is 0, your frameRate is set too high for the amount of data you are collecting.

Keep some amount of idleLoops free because whenever you make a ReflectaFunctions call, it takes time out of idleLoops.  If your ReflectaFunctions work takes more time than you have available in idleLoops, your Heartbeat frame will be delayed and this will introduce 'jitter' (aka variation between delivery times) into your heartbeat.

## Cooperative Multitasking ##

Arduino's loop() design is essentially a 'co-operative multitasking' approach.  Basically what this means is if you design efficient async functions to be called from loop(), you can share the 16 MHz CPU to do an amazing amount of work.  However it's simple to write code that just doesn't return but just spins the CPU while waiting for an IO to complete and this causes problems with this approach.  Two good examples of this (elaborate later) are the analogRead(...) method and the Pololu IMU I2C code.

One way to work around this is to use interrupts which are basically the microcontroller equivalent of 'pre-emptive multitasking'.  However my experience so far is that using interrupts can be tricky because there are so few of them and because they interfere with each other.  For example, if you use an higher priority interrupt to read the ADC, you might cause the Servo PulseOut to be interrupted causing jitter in that signal which makes the servos wiggle.  Because of this, Reflecta Heartbeats (and Arduino loop() in general) is designed to enable you to write functions that co-operatively share the CPU without interrupting each other or requiring interrupts to be temporarily disabled.