#Reflecta Samples

## LedButton

This is a very simple sample that demonstrates how to control an Arduino's LED from a web page. When you click on the browser button, it sends a message via socket.io to a node.js server. Node.js then uses Reflecta to talk to an Arduino via its serial port and toggle its built in LED.
## TrendyCompass

TrendyCompass is my attempt at smushing together every potential buzzword into one awesome sample.  Imagine a NodeJS powered server reading an Arduino Compass using Socket.IO over HTML5 WebSockets and Canvas to draw a compass arrow.

For all that trendy goodness, the arrow doesn't actually point in the right direction yet because I haven't written the code to find the gravitational reference acceleration and turn the 3D compass reading into the proper 2D heading.  However it does move randomly based on the X axis reading of the magnometer so the trendiness makes up for the overall inaccuracy, right?

Assumes an Arduino running the [BasicIMU](https://github.com/JayBeavers/Reflecta/tree/master/Samples/BasicIMU) sketch with a [Pololu MinIMU-9 v2](http://www.pololu.com/catalog/product/1268) properly attached to its I2C port.  Prototyped on a Teensy so pin configuration of BasicIMU may need to be adjusted for other Arduino models.

## moto1

Simple sample written to test the MOTO1 interface.

## queryInterface

Prints out the interfaces discovered on the Arduino by QueryInterface.

## readImu

Prints out IMU readings to the console.

Assumes an Arduino running the [BasicIMU](https://github.com/JayBeavers/Reflecta/tree/master/Samples/BasicIMU) sketch with a [Pololu MinIMU-9 v2](http://www.pololu.com/catalog/product/1268) properly attached to its I2C port.  Prototyped on a Teensy so pin configuration of BasicIMU may need to be adjusted for other Arduino models.

## simple

Simple example that turns on the LED of a Teensy (digital pin 11).  Can be used as a simple starting point for REPL to test your firmware:

    node
    > .load simple.js