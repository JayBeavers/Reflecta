#define TWI_FREQ 400000L
#include <Wire.h>
#include <L3G.h>
#include <LSM303.h>
#include <ReflectaFramesSerial.h>
#include <ReflectaFunctions.h>
#include <ReflectaHeartbeat.h>

L3G gyro;
LSM303 compass;

void setup()
{ 
  I2C_Init();
  Accel_Init();
  Compass_Init();
  Gyro_Init();
  
  reflectaFrames::setup(9600);
  reflectaFunctions::setup();
  
  reflectaHeartbeat::setFrameRate(1);
  
  reflectaHeartbeat::bind(readGyroscope);
  reflectaHeartbeat::bind(readAccelerometer);
  reflectaHeartbeat::bind(readMagnometer);
}

bool readGyroscope() {
  gyro.read();
  reflectaHeartbeat::pushf(gyro.g.x);
  reflectaHeartbeat::pushf(gyro.g.y);
  reflectaHeartbeat::pushf(gyro.g.z);
  return true;
}

bool readAccelerometer() {
  compass.readAcc();
  reflectaHeartbeat::pushf(compass.a.x);
  reflectaHeartbeat::pushf(compass.a.y);
  reflectaHeartbeat::pushf(compass.a.z);
  return true;
}

bool readMagnometer() {
  compass.readMag();
  reflectaHeartbeat::pushf(compass.m.x);
  reflectaHeartbeat::pushf(compass.m.y);
  reflectaHeartbeat::pushf(compass.m.z);
  return true;
}

void loop() //Main Loop
{
  reflectaFrames::loop();
  reflectaHeartbeat::loop();
}

void I2C_Init()
{
  Wire.begin();
  TWBR = 12; // set the I2C channel to 400 khz from 100 khz (TWBR == 72)
}

void Gyro_Init()
{
  gyro.init();
  gyro.writeReg(L3G_CTRL_REG1, 0xCF); // normal power mode, all axes enabled, 760 Hz
}

void Accel_Init()
{
  compass.init();
  compass.writeAccReg(LSM303_CTRL_REG1_A, 0x77); // normal power mode, all axes enabled, 400 Hz
}

void Compass_Init()
{
  compass.writeMagReg(LSM303_CRA_REG_M, 0x10); // 1C == 220 hz, 10 == 15 hz
  compass.writeMagReg(LSM303_CRB_REG_M, 0x20); // Lowest gain
  compass.writeMagReg(LSM303_MR_REG_M, 0x00); // continuous conversion mode
}

