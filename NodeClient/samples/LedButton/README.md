# Reflecta LED Button Sample #

This is a very simple sample that demonstrates how to control an Arduino's LED from a web page.  When you click on the browser button, it sends a message via socket.io to a node.js server.  Node.js then uses Reflecta to talk to an Arduino via its serial port and toggle its built in LED.

Note: This sample was written for a [Teensy](http://www.pjrc.com/teensy/) which has a different LED pin (pin 11) than an Uno (pin 13).

To use this sample, you must flash your Arduino with a sketch like [BasicReflecta](https://github.com/JayBeavers/Reflecta/tree/master/BasicReflecta) that contains ReflectaArduinoCore.  ReflectaArduinoCore contains the code that remotes digitalWrite which is used to toggle the LED.

Run 'npm install' to download and install dependency libraries like socket-io (used for browser->node communications) and serialport (used for node->Arduino communications).

Run 'node server.js' to start the server app.

Open your browser to 'http://localhost:8088' to load the client.