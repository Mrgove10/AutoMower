#include <Arduino.h>
#include <Adafruit_HMC5883_U.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Compass/Compass.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * I2C HMC5883L Compasss Sensor Setup function
 *
 */
void CompassSensorSetup()
{
  if (!Compass.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    DebugPrintln("Compass Sensor not found !", DBG_VERBOSE, false);
    LogPrintln("Compass Sensor not found !", TAG_CHECK, DBG_ERROR);
  }
  else
  {
    DebugPrintln("Compass Sensor found !", DBG_VERBOSE, false);
  }
  //  mag.setMagGain(HMC5883_MAGGAIN_4_0);

  /* Display some basic information on this sensor */
  DisplayCompassDetails();
}

/**
 * Function to test and read I2C HMC5883L Compass Sensor
 *  
 * @param Now true to force an immediate read
 */
void CompassRead(const bool Now)
{
#ifdef COMPASS_PRESENT

  static unsigned long LastCompassRead = 0;

  if ((millis() - LastCompassRead > COMPASS_READ_INTERVAL) || Now)
  {
    sensors_event_t event;
    //    Serial.println("Before getEvent");
    bool status = Compass.getEvent(&event);
    //    Serial.println("getEvent Satus:" + String(status));

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
                 DBG_INFO, true);

    LastCompassRead = millis();
  }
#endif
}

/**
 * Checks to see if I2C HMC5883L Compasss Sensor is connected (and hopefully functionning)
 * @return true if current sensor is ok
 */
bool CompassSensorCheck(void)
{
  CompassRead(true);
  sensor_t sensor;
#ifdef COMPASS_PRESENT
  Compass.getSensor(&sensor);
#endif
  DisplayClear();
  DisplayPrint(0, 0, F("Compass sensor Test"));

  if (sensor.sensor_id == COMPASS_ID)
  {
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
