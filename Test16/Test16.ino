#include <Arduino.h>
#include <ReflectaFramesSerial.h>
#include <ReflectaFunctions.h>

// Echo test firmware for validating the bit-correctness of push/pop serialization on MCU and with host
// To become unit tests eventually.

void echo1()
{
  byte in = reflectaFunctions::pop();
  reflectaFrames::sendMessage(String(in));
  reflectaFunctions::sendResponse(1, (byte*)&in);
}

void echo2()
{
  int16_t in = reflectaFunctions::pop16();
  reflectaFrames::sendMessage(String(in));

  reflectaFunctions::push16(in);
  int16_t in2 = reflectaFunctions::pop16();
  reflectaFrames::sendMessage(String(in2));

  reflectaFunctions::push16(in);
}

void echo8()
{
  byte in1 = reflectaFunctions::pop();
  byte in2 = reflectaFunctions::pop();
  byte in3 = reflectaFunctions::pop();
  byte in4 = reflectaFunctions::pop();
  reflectaFrames::sendMessage(String(in1) + " : " + String(in2) + " : " + String(in3) + " : " + String(in4));
  
  byte buffer[8];
  buffer[0] = ((byte*)&in1)[0];
  buffer[1] = ((byte*)&in1)[1];
  buffer[2] = ((byte*)&in2)[0];
  buffer[3] = ((byte*)&in2)[1];
  buffer[4] = ((byte*)&in3)[0];
  buffer[5] = ((byte*)&in3)[1];
  buffer[6] = ((byte*)&in4)[0];
  buffer[7] = ((byte*)&in4)[1];
  
  reflectaFunctions::sendResponse(8, buffer);
}

void setup()
{
  reflectaFrames::setup(9600);
  reflectaFunctions::setup();
  
  reflectaFunctions::bind("ECHO1", echo1);
  reflectaFunctions::bind("ECHO1", echo2);
  reflectaFunctions::bind("ECHO1", echo8);  
}

void loop()
{
  reflectaFrames::loop();
}