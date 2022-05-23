#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "i2c_definitions.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "GyroAccel/GyroAccel.h"
#include "EEPROM/EEPROM.h"

// This code is adapted from 
// https://electronoobs.com/images/Robotica/tut_9/MPU6050_acc_read_example.zip, and 
// Joop Brokking Website: http://www.brokking.net/imu.html

/**
 * GY-521 MPU6050 Setup function
 *
 */
void GyroAccelSetup()
{
  uint8_t I2CError;

  // Ensure exclusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  Wire.begin();                                      // begin the wire communication
  
  Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);  // begin, Send the sensor address
  Wire.write(0x6B);                                  // make the reset (place a 0 into the 6B register)
  Wire.write(0x00);
  Wire.endTransmission(true);                        // end the transmission

  //Gyro config
  Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);  // begin, Send the sensor address
  Wire.write(0x1B);                                  // We want to write to the GYRO_CONFIG register (1B hex)
  Wire.write(0x08);                                  // Set the register bits as 00001000 (500dps full scale)
  Wire.endTransmission(true);                        // End the transmission with the gyro
  
  //Acc config
  Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);  // Start communication with sensor
  Wire.write(0x1C);                                  // We want to write to the ACCEL_CONFIG register
  Wire.write(0x10);                                  // Set the register bits as 00010000 (+/- 8g full scale range)
  I2CError = Wire.endTransmission(true); 

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  if (I2CError == I2C_ERROR_OK)
  {
    DebugPrintln("Gyro/Accel setup Done", DBG_VERBOSE, true);
    g_GyroPresent = true;
  } 
  else
  {
    DebugPrintln("Gyro sensor not found ! (" + String(I2CError) + ")", DBG_ERROR, true);
    LogPrintln("Gyro Sensor not found", TAG_CHECK, DBG_ERROR);

    g_GyroPresent = false;
  }
}

/**
 * GY-521 MPU6050 Gyroscope and Acceleration calibration function
 * @param number of samples to establish calibration
 *
 */
// void AccelErrorCalibration(const int samples)
// {
//   // to establish acceleration error, we take the average error for X and Y

//   // Variables to store the raw data read from sensor
//   float AccelRawX = 0;
//   float AccelRawY = 0;
//   float AccelRawZ = 0;

//   // Variables to store accumulated error
//   float totalAngleErrorX = 0;
//   float totalAngleErrorY = 0;

//   DebugPrintln("Start of accel calibration on " + String(samples) + " samples", DBG_VERBOSE, true);

//   // Ensure exclusive access to I2C
//   xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

//   for(int i = 0; i < samples; i ++)
//   {
//     Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);
//     Wire.write(0x3B);                               //Ask for the 0x3B register- correspond to AcX
//     Wire.endTransmission(false);
//     Wire.requestFrom(GYRO_MPU6050_I2C_ADDRESS, 6, 1); 
    
//     AccelRawX=(Wire.read() << 8 | Wire.read()) / 4096.0 ; //each value needs two registers
//     AccelRawY=(Wire.read() << 8 | Wire.read()) / 4096.0 ;
//     AccelRawZ=(Wire.read() << 8 | Wire.read()) / 4096.0 ;

//     totalAngleErrorX = totalAngleErrorX + ((atan((AccelRawY)/sqrt(pow((AccelRawX),2) + pow((AccelRawZ),2))) * RAD_TO_DEG));
//     totalAngleErrorY = totalAngleErrorY + ((atan(-1*(AccelRawX)/sqrt(pow((AccelRawY),2) + pow((AccelRawZ),2))) * RAD_TO_DEG));

//     delay(5);
//   }

//   // Free access to I2C for other tasks
//   xSemaphoreGive(g_I2CSemaphore);

//   g_AccelAngleErrorX = totalAngleErrorX / samples;
//   g_AccelAngleErrorY = totalAngleErrorY / samples;

//   LogPrintln("Accel calibration done (X:" + String(g_AccelAngleErrorX, 1) + ", Y:" + String(g_AccelAngleErrorY, 1) + ")", TAG_VALUE, DBG_INFO);

//   // Reset total angle value
//   g_GyroAccelAngleX = 0;
//   g_GyroAccelAngleY = 0;
  
//   // Save calibration to EEPROM
//   EEPROMSave(true);
// }

/**
 * GY-521 MPU6050 Gyroscope error calibration function
 *
 * @param number of samples to establish calibration
 */
void GyroErrorCalibration(const int samples)
{
  // to establish gyroscope error, we take the average error for X and Y

  // Variables to store the raw data read from sensor
  float gyroRawXtotal = 0;
  float gyroRawYtotal = 0;
  float gyroRawZtotal = 0;
  float accelRawXtotal = 0;
  float accelRawYtotal = 0;
  float MPUTemptotal = 0;

  // Reset calibration values as they are being used in data read function
  g_GyroErrorX = 0;
  g_GyroErrorY = 0;
  g_GyroErrorZ = 0;
  g_AccelErrorX = 0;
  g_AccelErrorY = 0;
  g_MPUCalibrationTemperature = 0;

  if (g_GyroPresent)                         // Check that device is present
  { 
    DebugPrintln("Start of gyro calibration on " + String(samples) + " samples", DBG_VERBOSE, true);

    for(int i = 0; i < samples; i ++)
    {
      int16_t AccelRawX;
      int16_t AccelRawY;
      int16_t AccelRawZ;
      int16_t MPUTemperatureRaw;
      int16_t GyroRawX;
      int16_t GyroRawY;
      int16_t GyroRawZ;

      // Read the data from the sensor
      // Ensure exclusive access to I2C
      xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

      Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);     //begin, Send the slave address
      Wire.write(0x3B);                                     //Ask for the 0x3B register- correspond to AcX
      Wire.endTransmission(false);                          //keep the transmission and next
      Wire.requestFrom(GYRO_MPU6050_I2C_ADDRESS, 14, 1);    //We ask for next 6 registers starting with the 3B  
      while(Wire.available() < 14);                         //Wait until all the bytes are received

      AccelRawX = (Wire.read() << 8 | Wire.read());
      AccelRawY = (Wire.read() << 8 | Wire.read());
      AccelRawZ = (Wire.read() << 8 | Wire.read());
      MPUTemperatureRaw = (Wire.read() << 8 | Wire.read());
      GyroRawX = (Wire.read() << 8 | Wire.read());
      GyroRawY = (Wire.read() << 8 | Wire.read());
      GyroRawZ = (Wire.read() << 8 | Wire.read());

      // Free access to I2C for other tasks
      xSemaphoreGive(g_I2CSemaphore);

    // Serial.println("X:" + String(GyroRawX) + " Y:" + String(GyroRawY) +" Z:" + String(GyroRawZ) + " Temp: " + String(MPUTemperatureRaw));

      GyroAccelDataRead();

      // Sum the values
      gyroRawXtotal = gyroRawXtotal + GyroRawX;
      gyroRawYtotal = gyroRawYtotal + GyroRawY;
      gyroRawZtotal = gyroRawZtotal + GyroRawZ;

      accelRawXtotal = accelRawXtotal + AccelRawX;
      accelRawYtotal = accelRawYtotal + AccelRawY;
      
      MPUTemptotal = MPUTemptotal + float(MPUTemperatureRaw) / 340.0f + 36.53f;

      delay(10);
    }

    g_GyroErrorX = gyroRawXtotal / samples;
    g_GyroErrorY = gyroRawYtotal / samples;
    g_GyroErrorZ = gyroRawZtotal / samples;

    g_AccelErrorX = accelRawXtotal / samples;
    g_AccelErrorY = accelRawYtotal / samples;

    g_MPUCalibrationTemperature = MPUTemptotal / samples;
    
    LogPrintln("Gyro calibration done (GX:" + String(g_GyroErrorX, 5) + 
              ", GY:" + String(g_GyroErrorY, 5) + 
              ", GZ:" + String(g_GyroErrorZ, 5) + 
              ", AX:" + String(g_AccelErrorX, 5) +
              ", AY:" + String(g_AccelErrorY, 5) + 
              ", MPUTemp:" + String(g_MPUCalibrationTemperature, 2) + 
              ")", TAG_VALUE, DBG_INFO);

    // Reset total angle value
    g_pitchAngle = 0;
    g_rollAngle = 0;

    // Save calibration to EEPROM
    EEPROMSave(true);

    // Reset value calculations
    PitchRollCalc(true, true);
  }
  else
  {
    DebugPrintln("Gyro sensor not present !", DBG_ERROR, true);
  }
}

/**
 * GY-521 MPU6050 gyroscope and Acceleration raw values read function
 * @return true if read successful, false if sensor not found
 *
 */
bool GyroAccelDataRead(void)
{
  int16_t AccelRawX;
  int16_t AccelRawY;
  int16_t AccelRawZ;
  int16_t MPUTemperatureRaw;
  int16_t GyroRawX;
  int16_t GyroRawY;
  int16_t GyroRawZ;

  static unsigned long lastReadTime = millis();

  if (g_GyroPresent)                         // Check that device is present
  {
    // Ensure exclusive access to I2C
    xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

    Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);     //begin, Send the slave address
    Wire.write(0x3B);                                     //Ask for the 0x3B register- correspond to AcX
    Wire.endTransmission(false);                          //keep the transmission and next
    Wire.requestFrom(GYRO_MPU6050_I2C_ADDRESS, 14, 1);    //We ask for next 6 registers starting with the 3B  
    while(Wire.available() < 14);                         //Wait until all the bytes are received

    AccelRawX = (Wire.read() << 8 | Wire.read());
    AccelRawY = (Wire.read() << 8 | Wire.read());
    AccelRawZ = (Wire.read() << 8 | Wire.read());
    MPUTemperatureRaw = (Wire.read() << 8 | Wire.read());
    GyroRawX = (Wire.read() << 8 | Wire.read());
    GyroRawY = (Wire.read() << 8 | Wire.read());
    GyroRawZ = (Wire.read() << 8 | Wire.read());

    unsigned long readTime = millis();

    // Free access to I2C for other tasks
    xSemaphoreGive(g_I2CSemaphore);

    // Serial.println("X:" + String(GyroRawX) + " Y:" + String(GyroRawY) +" Z:" + String(GyroRawZ) + " Temp: " + String(MPUTemperatureRaw));

    // Read values are adjusted to take into account sensor sensitivity (based on selected ranges) and calibration
    g_AccelRawX = float(AccelRawX - g_AccelErrorX) / 4096 ;
    g_AccelRawY = float(AccelRawY - g_AccelErrorY) / 4096 ;
    g_AccelRawZ = float(AccelRawZ) / 4096;
    g_MPUTemperature = float(MPUTemperatureRaw) / 340.0f + 36.53f;
    g_GyroRawX = (float(GyroRawX - g_GyroErrorX) / 65.5 * (float(readTime - lastReadTime) / 1000));
    g_GyroRawY = (float(GyroRawY - g_GyroErrorY) / 65.5 * (float(readTime - lastReadTime) / 1000));
    g_GyroRawZ = (float(GyroRawZ - g_GyroErrorZ) / 65.5 * (float(readTime - lastReadTime) / 1000));

    // Serial.println("\t\t\t\t\t\tAX:" + String(g_AccelRawX, 3) + " AY:" + String(g_AccelRawY, 3) +" AZ:" + String(g_AccelRawZ, 3) + " | GX:" + String(g_GyroRawX, 3) + " GY:" + String(g_GyroRawY, 3) +" GZ:" + String(g_GyroRawZ, 3) + " Temp:" + String(g_MPUTemperature, 2));

    lastReadTime = readTime;
    return true;
  }
  else
  {
    // DebugPrintln("Gyro sensor not found !", DBG_ERROR, true);
    return false;
  }
}

/**
 * GY-521 MPU6050 Pitch and Roll calculation function
 * @param Now true to force an immediate read
 * @param reset  true to force a pitch an roll reset
 * 
 */
void PitchRollCalc(const bool Now, const bool reset)
{
  static float pitchAngle = 0;
  static float rollAngle = 0;
  static float AccelPitchAngle = 0;
  static float AccelrollAngle = 0;
  static bool initDone = false;

  static unsigned long LastPitchRollCalc = 0;

  #ifdef MQTT_PITCH_ROLL_DEBUG
  static unsigned long LastPitchRollSendTime = 0;
  #endif

  if (reset)
  {
    pitchAngle = 0;
    rollAngle = 0;
  }
  
  if (g_MotionMotorTurnInProgress)
  {
    GyroAccelDataRead();    // read the data du clear the gyro data, but doe not calculate values
  }
  else
  {
    if ((millis() - LastPitchRollCalc > GYRO_ACCEL_ANGLE_CALC_INTERVAL) || Now)
    {
      // Read latest sensor data
      if (GyroAccelDataRead()) 
      {
        // Calculate temperature compensation factors
        float pitchTemperatureCompensation = PITCH_TEMPERATURE_COMPENSATION_A * (g_MPUTemperature - g_MPUCalibrationTemperature) + PITCH_TEMPERATURE_COMPENSATION_B;
        float rollTemperatureCompensation = ROLL_TEMPERATURE_COMPENSATION_A * (g_MPUTemperature - g_MPUCalibrationTemperature) + ROLL_TEMPERATURE_COMPENSATION_B;
        
        // Calculate gyro angle values
        pitchAngle = pitchAngle + g_GyroRawX + g_GyroRawY * sin(g_GyroRawZ * DEG_TO_RAD);
        rollAngle =  rollAngle + g_GyroRawY - g_pitchAngle * sin(g_GyroRawZ * DEG_TO_RAD);

        // Accelerometer angle calculations
        float accelTotalVector = sqrt((g_AccelRawX * g_AccelRawX) + (g_AccelRawY * g_AccelRawY) + (g_AccelRawZ * g_AccelRawZ));  //Calculate the total accelerometer vector
        AccelPitchAngle = asin(g_AccelRawY / accelTotalVector) * RAD_TO_DEG;
        AccelrollAngle = asin(g_AccelRawX / accelTotalVector) * -RAD_TO_DEG;

        // If the Sensor is already started
        if (initDone && !reset)
        {
          pitchAngle = pitchAngle * 0.99 + AccelPitchAngle * 0.01;  //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
          rollAngle = rollAngle * 0.99 + AccelrollAngle * 0.01;     //Correct the drift of the gyro roll angle with the accelerometer roll angle
        }
        // At first start
        else
        {
          pitchAngle = AccelPitchAngle;   // Set the gyro pitch angle equal to the accelerometer pitch angle 
          rollAngle = AccelrollAngle;     // Set the gyro roll angle equal to the accelerometer roll angle
          initDone = true;
        }

        // To dampen the pitch and roll angles a complementary filter is used
        g_pitchAngle = g_pitchAngle * 0.7 + pitchAngle * 0.3;
        g_rollAngle = g_rollAngle * 0.7 + rollAngle * 0.3;

        g_TCpitchAngle = g_pitchAngle - pitchTemperatureCompensation;
        g_TCrollAngle = g_rollAngle - rollTemperatureCompensation;

        LastPitchRollCalc = millis();

        // DebugPrintln("Pitch: " + String(g_pitchAngle, 3) + ", Roll:" + String(g_rollAngle, 3), DBG_VERBOSE, true);
      }
      else
      {
        pitchAngle = 0;
        rollAngle = 0;
        // DebugPrintln("Pitch & Roll reset - Sensor not found!", DBG_VERBOSE, true);
        LastPitchRollCalc = millis();
      }
    }
  }

  #ifdef MQTT_PITCH_ROLL_DEBUG
    //  send debug information through MQTT
    if (g_MQTTPitchRollDebug && millis() - LastPitchRollSendTime > MQTT_PITCH_ROLL_DEBUG_CHANNEL_INTERVAL)
    {
      String JsonPayload = "";
      FirebaseJson JSONDBGPayload;
      String JSONDBGPayloadStr;
      char MQTTpayload[MQTT_MAX_PAYLOAD];

      JSONDBGPayload.clear();
      JSONDBGPayload.add("P", g_pitchAngle);
      JSONDBGPayload.add("R", g_rollAngle);
      JSONDBGPayload.add("CTP", g_TCpitchAngle);
      JSONDBGPayload.add("CTR", g_TCrollAngle);
      JSONDBGPayload.add("T", g_MPUTemperature);
      JSONDBGPayload.toString(JSONDBGPayloadStr, false);
      JSONDBGPayloadStr.toCharArray(MQTTpayload, JSONDBGPayloadStr.length() + 1);
      bool result = MQTTclient.publish(MQTT_PITCH_ROLL_DEBUG_CHANNEL, MQTTpayload);
      if (result != 1)
      {
        g_MQTTErrorCount = g_MQTTErrorCount + 1;
      }
  	  LastPitchRollSendTime = millis();

      MQTTclient.loop();
      //    DebugPrintln("Sending to :[" + String(MQTT_PITCH_ROLL_DEBUG_CHANNEL) + "] " + String(MQTTpayload) + " => " + String(result), DBG_VERBOSE, true);
    }
#endif

}
/**
 * GY-521 MPU6050 Gyro Angle value read function
 *
 */
// void GyroAngleRead(void)
// {
//   static unsigned long lastReadTime = millis();
//   float GyroRawX = 0;
//   float GyroRawY = 0;

//   // Ensure exclusive access to I2C
//   xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

//   Wire.beginTransmission(GYRO_MPU6050_I2C_ADDRESS);            //begin, Send the slave addrress
//   Wire.write(0x43);                                            //First address of the Gyro data
//   Wire.endTransmission(false);
//   Wire.requestFrom(GYRO_MPU6050_I2C_ADDRESS, 4, 1);           //We ask for just 4 registers

//   unsigned long readTime = millis();

//   GyroRawX = Wire.read() << 8 | Wire.read();     //Once again we shift and sum
//   GyroRawY = Wire.read() << 8 | Wire.read();

//   // Free access to I2C for other tasks
//   xSemaphoreGive(g_I2CSemaphore);

//   /*Now in order to obtain the gyro data in degrees/seconds we have to divide first
//   the raw value by 32.8 because that's the value that the datasheet gives us for a 1000dps range*/
//   GyroRawX = (GyroRawX / 32.8) - g_GyroErrorX; 
//   GyroRawY = (GyroRawY / 32.8) - g_GyroErrorY;

//   /*Now we integrate the raw value in degrees per seconds in order to obtain the angle
//   * If you multiply degrees/seconds by seconds you obtain degrees */

//   g_GyroAngleX = GyroRawX * (readTime - lastReadTime) / 1000;
//   g_GyroAngleY = GyroRawY * (readTime - lastReadTime) / 1000;

//   lastReadTime = readTime;

//   DebugPrintln("Gyro angle: [X:" + String(g_GyroAngleX, 1) + ", Y:" + String(g_GyroAngleY, 1) + "]" + " in " + String(readTime - lastReadTime) + " ms", DBG_VERBOSE, true);
// }

/**
 * GY-521 MPU6050 Combined Gyro/Accel Angle read function
 * 
 * @param Now true to force an immediate read
 */
// void GyroAccelAngleRead(const bool Now)
// {
//   static unsigned long LastGyroAccelAngleRead = 0;

//   if ((millis() - LastGyroAccelAngleRead > GYRO_ACCEL_ANGLE_READ_INTERVAL) || Now)
//   {
//     GyroAngleRead();
//     AccelAngleRead();

//     g_GyroAccelAngleX = 0.98 * (g_GyroAccelAngleX + g_GyroAngleX) + 0.02 * g_AccelAngleX;
//     g_GyroAccelAngleY = 0.98 * (g_GyroAccelAngleY + g_GyroAngleY) + 0.02 * g_AccelAngleY;

//     DebugPrintln("Total angles: [X:" + String(g_GyroAccelAngleX, 1) + ", Y:" + String(g_GyroAccelAngleY, 1) + "]", DBG_VERBOSE, true);
    
//     LastGyroAccelAngleRead = millis();
//   }
// }

/**
 * Checks to see if Gyro/Accel is connected (and hopefully functioning)
 * @return true if is ok
 */
bool GyroAccelCheck(void)
{
  PitchRollCalc(true);

  DisplayClear();
  DisplayPrint(0, 0, F("Gyro/Accel Test"));

  if (g_GyroPresent)
  {
    DisplayPrint(2, 2, "Pitch:" + String(g_TCpitchAngle, 1));
    DisplayPrint(2, 3, "Roll:" + String(g_TCrollAngle, 1));
  }
  else
  {
    DisplayPrint(2, 2, F("Sensor ERROR"));
  }

  delay(TEST_SEQ_STEP_WAIT);
  return true;
}
