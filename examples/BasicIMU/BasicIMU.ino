#include <Wire.h>
#include <Servo.h>
#include <L3G.h>
#include <LSM303.h>
#include <Reflecta.h>

L3G gyro;
LSM303 compass;

void setup()
{ 
  I2C_Init();
  Compass_Init();
  Gyro_Init();
  
  reflectaFrames::setup(9600);
  reflectaFunctions::setup();
  reflectaArduinoCore::setup();
  reflectaHeartbeat::setup();
  
  reflectaFunctions::push16(1);
  reflectaHeartbeat::setFrameRate();
  
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

void Compass_Init()
{
  compass.init(LSM303DLHC_DEVICE);
  compass.enableDefault();
  compass.writeAccReg(LSM303_CTRL_REG1_A, 0x27); // Bump accelerometer from 50 hz to 400 hz
  
  // Lowest possible gain value that doesn't overflow and return -4096
  compass.setMagGain(LSM303::magGain_40);
  
  // Calibration values, use Calibrate example program to get the values for your compass.
  compass.m_min.x = 1821; compass.m_min.y = -1770; compass.m_min.z = 904;
  compass.m_max.x = 2030; compass.m_max.y = -1396; compass.m_max.z = 1161;
}
