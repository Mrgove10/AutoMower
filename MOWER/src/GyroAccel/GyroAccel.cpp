#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "i2c_definitions.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "GyroAccel/GyroAccel.h"

// This code is adapted from https://electronoobs.com/images/Robotica/tut_9/MPU6050_acc_read_example.zip

/**
 * GY-521 MPU6050 Setup function
 *
 */
void GyroAccelSetup()
{
  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  Wire.begin();                                      // begin the wire comunication
  
  Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);  // begin, Send the sensor adress
  Wire.write(0x6B);                                  // make the reset (place a 0 into the 6B register)
  Wire.write(0x00);
  Wire.endTransmission(true);                        // end the transmission

  //Gyro config
  Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);  // begin, Send the sensor adress
  Wire.write(0x1B);                                  // We want to write to the GYRO_CONFIG register (1B hex)
  Wire.write(0x10);                                  // Set the register bits as 00010000 (1000dps full scale)
  Wire.endTransmission(true);                        // End the transmission with the gyro
  
  //Acc config
  Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);  // Start communication with sensor
  Wire.write(0x1C);                                  // We want to write to the ACCEL_CONFIG register
  Wire.write(0x10);                                  // Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true); 

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  DebugPrintln("Gyro/Accel setup Done", DBG_VERBOSE, true);

  // Calibrate Accelerometer and Gyroscope

  AccelErrorCalibration(500);
  GyroErrorCalibration(500);
}

/**
 * GY-521 MPU6050 Acceleration error calibration function
 * @param number of samples to establish calibration
 *
 */
void AccelErrorCalibration(const int samples)
{
  // to establish acceleration error, we take the average error for X and Y

  // Variables to store the raw data read from sensor
  float AccelRawX = 0;
  float AccelRawY = 0;
  float AccelRawZ = 0;

  // Variables to store accumulated error
  float totalAngleErrorX = 0;
  float totalAngleErrorY = 0;

  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  for(int i = 0; i < samples; i ++)
  {
    Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);
    Wire.write(0x3B);                               //Ask for the 0x3B register- correspond to AcX
    Wire.endTransmission(false);
    Wire.requestFrom(GYRO_MPU6050_I2C_ADDRESS, 6, 1); 
    
    AccelRawX=(Wire.read()<<8|Wire.read())/4096.0 ; //each value needs two registres
    AccelRawY=(Wire.read()<<8|Wire.read())/4096.0 ;
    AccelRawZ=(Wire.read()<<8|Wire.read())/4096.0 ;

    totalAngleErrorX = totalAngleErrorX + ((atan((AccelRawY)/sqrt(pow((AccelRawX),2) + pow((AccelRawZ),2))) * RAD_TO_DEG));
    totalAngleErrorY = totalAngleErrorY + ((atan(-1*(AccelRawX)/sqrt(pow((AccelRawY),2) + pow((AccelRawZ),2))) * RAD_TO_DEG));

    delay(5);
  }

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  g_AccelAngleErrorX = totalAngleErrorX / samples;
  g_AccelAngleErrorY = totalAngleErrorY / samples;

  DebugPrintln("Accel calibration done (X:" + String(g_AccelAngleErrorX,1) + ", Y:" + String(g_AccelAngleErrorY,1) + ")", DBG_INFO, true);
}

/**
 * GY-521 MPU6050 Gyroscope error calibration function
 *
 * @param number of samples to establish calibration
 */
void GyroErrorCalibration(const int samples)
{
  // to establish gyroscope error, we take the average error for X and Y

  // Variables to store the raw data read from sensor
  float gyroRawX = 0;
  float gyroRawY = 0;

  // Variables to store accumulated error
  float totalGyroErrorX = 0;
  float totalGyroErrorY = 0;

  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  for(int i = 0; i < samples; i ++)
  {
    Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);            // begin, Send the slave adress 
    Wire.write(0x43);                                            // First adress of the Gyro data
    Wire.endTransmission(false);
    Wire.requestFrom(GYRO_MPU6050_I2C_ADDRESS, 4, 1);           // We ask for just 4 registers 

    gyroRawX=Wire.read()<<8|Wire.read();     //Once again we shif and sum
    gyroRawY=Wire.read()<<8|Wire.read();

    totalGyroErrorX = totalGyroErrorX + (gyroRawX/32.8); 
    totalGyroErrorY = totalGyroErrorY + (gyroRawY/32.8);

    delay(5);
  }

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  g_GyroErrorX = totalGyroErrorX / samples;
  g_GyroErrorY = totalGyroErrorY / samples;
  DebugPrintln("Gyro calibration done (X:" + String(g_GyroErrorX,1) + ", Y:" + String(g_GyroErrorY,1) + ")", DBG_INFO, true);
}
