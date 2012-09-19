#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <Reflecta.h>

// Very simple Arduino sketch for using reflecta.  Opens a COM port
// and exposes the Arduino Core functions (pinMode, digitalRead, etc.)
void setup()
{
  reflecta::setup(57200);
}

void loop()
{
  reflecta::loop();
}
