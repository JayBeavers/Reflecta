#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <ReflectaFramesSerial.h>
#include <ReflectaFunctions.h>
#include <ReflectaArduinoCore.h>

// Very simple Arduino sketch for using reflecta.  Opens a COM port
// and exposes the ArduinoCore functions (pinMode, digitalRead, etc.)

void setup()
{
  reflectaFrames::setup(9600);
  reflectaFunctions::setup();
  reflectaArduinoCore::setup();
}

void loop()
{
  reflectaFrames::loop();
}