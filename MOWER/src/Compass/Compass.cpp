#include <Arduino.h>
#include <Adafruit_HMC5883_U.h>
#include "i2c_definitions.h"
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Compass/Compass.h"
#include "MotionMotor/MotionMotor.h"
#include "CutMotor/CutMotor.h"
#include "EEPROM/EEPROM.h"
#include "Buzzer/Buzzer.h"
#include "MQTT/MQTT.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * I2C HMC5883L Compasss Sensor Setup function
 *
 */
void CompassSensorSetup()
{
  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  // Check if Compass is accessible on I2C bus
  Wire.beginTransmission(COMPASS_HMC5883L_I2C_ADDRESS);
  Wire.write(HMC5883_REGISTER_MAG_OUT_X_H_M);
  uint8_t I2CError = Wire.endTransmission();

  g_CompassPresent = (I2CError == I2C_ERROR_OK);

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  if (!g_CompassPresent)
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    DebugPrintln("Compass Sensor not found !", DBG_VERBOSE, false);
    LogPrintln("Compass Sensor not found !", TAG_CHECK, DBG_ERROR);
  }
  else
  {
    DebugPrintln("Compass Sensor found !", DBG_VERBOSE, false);

    I2C_write_AddrDev_AddrReg_Byte(COMPASS_HMC5883L_I2C_ADDRESS,0x00,0x70);      // CRA register @ (8-average, 15 Hz default, normal measurement)
    I2C_write_AddrDev_AddrReg_Byte(COMPASS_HMC5883L_I2C_ADDRESS,0x02,0x00);      // MR registery @ 0 for continuous mode
    I2C_write_AddrDev_AddrReg_Byte(COMPASS_HMC5883L_I2C_ADDRESS,0x01,B100000);   // CRB registery @ 32 for 1.3Ga gain
  }
  //  mag.setMagGain(HMC5883_MAGGAIN_4_0);

  /* Display some basic information on this sensor */
  DisplayCompassDetails();
}

/**
 * @brief Function to read I2C HMC5883L Compass Sensor values
 *  
 * @param magX pointer to float used to store X magnitude value (output)
 * @param magY pointer to float used to store Y magnitude value (output)
 * @param magZ pointer to float used to store Z magnitude value (output)
 * @param Calibrated true to return calibrated value. If false, raw values are returned
 */
void getCompassValues(float *magX, float *magY, float *magZ, const bool Calibrated)
{
  int16_t mag_x, mag_y, mag_z;

  if (g_CompassPresent)
  {
    // Ensure exlusive access to I2C
    xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

    // Check if Compass is accessible on I2C bus
    Wire.beginTransmission(COMPASS_HMC5883L_I2C_ADDRESS);
    Wire.write(0x09);
    Wire.requestFrom((byte)COMPASS_HMC5883L_I2C_ADDRESS, (byte)1);
    int status = Wire.read(); // DOR Data out Ready (SKIPPED).
    Wire.endTransmission();

    if (status && 0x01)                     // Check data ready bit
    {
      Wire.beginTransmission(COMPASS_HMC5883L_I2C_ADDRESS);
      Wire.write(0x03);                                               // read from address 3 = x,y,z registers.
      Wire.endTransmission();
      Wire.requestFrom((byte)COMPASS_HMC5883L_I2C_ADDRESS, (byte)6); 
      while (Wire.available() < 6);                                  // Wait for the data

      uint8_t xhi = Wire.read();
      uint8_t xlo = Wire.read();
      uint8_t zhi = Wire.read();
      uint8_t zlo = Wire.read();
      uint8_t yhi = Wire.read();
      uint8_t ylo = Wire.read();

      // Shift values to create properly formed integer (low byte first)
      mag_x = (int16_t)(xlo | ((int16_t)xhi << 8));
      mag_y = (int16_t)(ylo | ((int16_t)yhi << 8));
      mag_z = (int16_t)(zlo | ((int16_t)zhi << 8));

      if (Calibrated)
      {
        *magX = (float(mag_x) - g_CompassMagXOffset) * g_CompassMagXScale;
        *magY = (float(mag_y) - g_CompassMagYOffset) * g_CompassMagYScale;
        *magZ = (float(mag_z) - g_CompassMagZOffset) * g_CompassMagZScale;
      }
      else
      {
        *magX = float(mag_x);
        *magY = float(mag_y);
        *magZ = float(mag_z);
      }
    }

    // Free access to I2C for other tasks
    xSemaphoreGive(g_I2CSemaphore);

    // DebugPrintln("Compass readings: X:" + String(*magX, COMPASS_PRECISION) +
    //             "(uT) Y:" + String(*magY, COMPASS_PRECISION) +
    //             "(uT) Z:" + String(*magZ, COMPASS_PRECISION) + "(uT)",
    //             DBG_VERBOSE, true);
  }
}

/**
 * @brief Function to calibrate compass for hard and soft iron effects
 *  
 * @param samples number of samples to base calibration on
 */
void CompassCalibrate(int samples)
{
  // Calibration is to be performed on a flat surface and while the mower is performing a number of "on the spot rotations" at a slow speed
  // Calling function is to check that conditions are met to perform a calibration (mower is Idle and no close objects)

  // Start to turn
  float turnDuration = float(abs(COMPASS_CALIBRATION_TURNS * 360) / ((MOWER_MOVES_TURN_ANGLE_RATIO / MOWER_MOVES_TURN_SPEED) * COMPASS_CALIBRATION_ROTATION_SPEED));
  DebugPrintln("Compass calibration Mower turn of " + String(COMPASS_CALIBRATION_TURNS * 360) + " Deg => " + String(turnDuration, 0) + " ms and " + String(samples) + " samples ", DBG_VERBOSE, true);
  
  // Disable pitch and roll calculation
  g_MotionMotorTurnInProgress = true;

  // Raw data
  float magX = 0;
  float magY = 0;
  float magZ = 0;

  // Raw data extremes
  float magXMin = 32767;
  float magXMax = -32768;
  float magYMin = 32767;
  float magYMax = -32768;
  float magZMin = 32767;
  float magZMax = -32768;
  float chord_x,  chord_y,  chord_z;                              // Used for calculating scale factors
  float chord_average;

  // Sound starting beep to notify environment
  playTune(g_longBeep, sizeof(g_longBeep) / sizeof(noteStruct), 3);

  // Start cut motor
  CutMotorStart(CUT_MOTOR_FORWARD, MOWER_MOWING_CUTTING_SPEED);
  // Give time for cut motor to start
  delay(MOWER_MOWING_CUT_START_WAIT);

  unsigned long TurnStartTime = millis();

  MotionMotorStart(MOTION_MOTOR_RIGHT, MOTION_MOTOR_FORWARD, COMPASS_CALIBRATION_ROTATION_SPEED);
  MotionMotorStart(MOTION_MOTOR_LEFT, MOTION_MOTOR_REVERSE, COMPASS_CALIBRATION_ROTATION_SPEED);

  int samplesRead = 0;
  // Collect calibration samples
  while(millis() - TurnStartTime < long(turnDuration) && samplesRead < COMPASS_CALIBRATION_SAMPLES)
  {
    getCompassValues(&magX, &magY, &magZ, false);

    magXMin = min(magX, magXMin);
    magXMax = max(magX, magXMax);
    magYMin = min(magY, magYMin);
    magYMax = max(magY, magYMax);
    magZMin = min(magZ, magZMin);
    magZMax = max(magZ, magZMax);

    samplesRead = samplesRead + 1;

    delay(4);                                                     // Time interval between magnetometer readings
  }

  // Stop cut motor
  CutMotorStop(true);

  // Stop rotation
  MotionMotorStop(MOTION_MOTOR_RIGHT);
  MotionMotorStop(MOTION_MOTOR_LEFT);
  g_MotionMotorTurnInProgress = false;

  // Calculate calibration parameters

  DebugPrintln("Compass calibration Min / Max for X: [" + String(magXMin, 0) + "," + String(magXMax, 0) + "]", DBG_VERBOSE, true);
  DebugPrintln("Compass calibration Min / Max for Y: [" + String(magYMin, 0) + "," + String(magYMax, 0) + "]", DBG_VERBOSE, true);
  DebugPrintln("Compass calibration Min / Max for Z: [" + String(magZMin, 0) + "," + String(magZMax, 0) + "]", DBG_VERBOSE, true);

  // hard-iron offsets
  g_CompassMagXOffset = (magXMax + magXMin) / 2;                     // Get average magnetic bias in counts
  g_CompassMagYOffset = (magYMax + magYMin) / 2;
  g_CompassMagZOffset = (magZMax + magZMin) / 2;

  // soft-iron scale factors
  chord_x = ((float)(magXMax - magXMin)) / 2;                 // Get average max chord length in counts
  chord_y = ((float)(magYMax - magYMin)) / 2;
  chord_z = ((float)(magZMax - magZMin)) / 2;

  chord_average = (chord_x + chord_y + chord_z) / 3;              // Calculate average chord length

  g_CompassMagXScale = chord_average / chord_x;                          // Calculate X scale factor
  g_CompassMagYScale = chord_average / chord_y;                          // Calculate Y scale factor
  g_CompassMagZScale = chord_average / chord_z;                          // Calculate Z scale factor

  MQTTReconnect();
  
  LogPrintln("Compass calibration done on " + String(samplesRead) + " samples (Offsets X:" + String(g_CompassMagXOffset, 1) + 
              ", Y:" + String(g_CompassMagYOffset, 1) + 
              ", Z:" + String(g_CompassMagZOffset, 1) + 
              ", Scale X:" + String(g_CompassMagXScale, 5) +
              ", Y:" + String(g_CompassMagYScale, 5) + 
              ", Z:" + String(g_CompassMagZScale, 5) + 
              ")", TAG_VALUE, DBG_INFO);

  MQTTclient.loop();

  // Save Calibration parmeters in EEPROM
  EEPROMSave(true);
}

/**
 * Function to test and read I2C HMC5883L Compass Sensor
 *  
 * @param Now true to force an immediate read
 */
void CompassRead(const bool Now)
{
  if (g_CompassPresent) 
  {
    static unsigned long LastCompassRead = 0;

    if ((millis() - LastCompassRead > COMPASS_READ_INTERVAL) || Now)
    {
      sensors_event_t event;
      
      // Ensure exlusive access to I2C
      xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);
    
      //    Serial.println("Before getEvent");
      bool status = Compass.getEvent(&event);
      //    Serial.println("getEvent Satus:" + String(status));

      // Free access to I2C for other tasks
      xSemaphoreGive(g_I2CSemaphore);

      // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
      // Calculate heading when the magnetometer is level, then correct for signs of axis.
      float heading = atan2(event.magnetic.y, event.magnetic.x);
      float headingCorr = atan2(event.magnetic.y + COMPASS_Y_HEADING_CORRECTION, event.magnetic.x + COMPASS_X_HEADING_CORRECTION);

      // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
      // Find yours here: http://www.magnetic-declination.com/
      // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
      // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
      heading += COMPASS_DECLINATION_ANGLE;
      headingCorr += COMPASS_DECLINATION_ANGLE;

      // Correct for when signs are reversed.
      if (heading < 0)
        heading += 2 * PI;
      if (headingCorr < 0)
        headingCorr += 2 * PI;

      // Check for wrap due to addition of declination.
      if (heading > 2 * PI)
        heading -= 2 * PI;
      if (headingCorr > 2 * PI)
        headingCorr -= 2 * PI;

      // Convert radians to degrees for readability.
      float headingDegrees = heading * 180 / M_PI;
      float headingDegreesCorr = headingCorr * 180 / M_PI;

      g_CompassHeading = headingDegrees;
      g_CompassHeadingCorrected = headingDegreesCorr;
      g_CompassXField = event.magnetic.x;
      g_CompassYField = event.magnetic.y;

      DebugPrintln("Compass readings: X:" + String(g_CompassXField, COMPASS_PRECISION) +
                      "(uT) Y:" + String(g_CompassYField, COMPASS_PRECISION) +
                      "(uT) Z:" + String(event.magnetic.z, COMPASS_PRECISION) + "(uT)",
                  DBG_VERBOSE, true);

      DebugPrintln("Compass Heading:" + String(g_CompassHeading, COMPASS_PRECISION) +
                      " Corrected: " + String(g_CompassHeadingCorrected, COMPASS_PRECISION),
                  DBG_DEBUG, true);

      LastCompassRead = millis();
    }
  }
}

/**
 * Checks to see if I2C HMC5883L Compasss Sensor is connected (and hopefully functionning)
 * @return true if current sensor is ok
 */
bool CompassSensorCheck(void)
{
  // // Check if Compass is accessible on I2C bus
  // Wire.beginTransmission(COMPASS_HMC5883L_I2C_ADDRESS);
  // Wire.write(HMC5883_REGISTER_MAG_OUT_X_H_M);
  // uint8_t I2CError = Wire.endTransmission();

  // g_CompassPresent = (I2CError == I2C_ERROR_OK);

  DisplayClear();
  DisplayPrint(0, 0, F("Compass sensor Test"));

  if (g_CompassPresent)
  {
    CompassRead(true);

    DebugPrintln("Compass Sensor ok", DBG_INFO, true);
    DisplayPrint(2, 2, "Sensor Ok");
    DisplayPrint(2, 3, "Heading: " + String(g_CompassHeading, 1));
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("Compass Sensor not found", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 2, "Sensor ERROR");
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Displays I2C HMC5883L Compasss Sensor details
 */
void DisplayCompassDetails(void)
{
  sensor_t sensor;
  Compass.getSensor(&sensor);
  DebugPrintln("------------------------------------", DBG_VERBOSE, true);
  DebugPrintln("Sensor:       " + String(sensor.name), DBG_VERBOSE, true);
  DebugPrintln("Driver Ver:   " + String(sensor.version), DBG_VERBOSE, true);
  DebugPrintln("Unique ID:    " + String(sensor.sensor_id), DBG_VERBOSE, true);
  DebugPrintln("Max Value:    " + String(sensor.max_value) + " uT", DBG_VERBOSE, true);
  DebugPrintln("Min Value:    " + String(sensor.min_value) + " uT", DBG_VERBOSE, true);
  DebugPrintln("Resolution:   " + String(sensor.resolution) + " uT", DBG_VERBOSE, true);
  DebugPrintln("------------------------------------");
  DebugPrintln("");
  delay(500);
}
