#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Sonar/Sonar.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Sonar sensor setup function
 * 
 */
void SonarSensorSetup(void)
{
  DebugPrintln("Sonar Sensor Setup start", DBG_VERBOSE, true);
}

/**
 * Checks to see if Sonar sensor is connected and functionning
 * @param sensor int Sonar to check
 * @return true if sensor check is ok
 */
bool SonarSensorCheck(int sensor)
{
  bool sensorCheck = false;
  unsigned int Distance = UNKNOWN_INT;

  DebugPrintln("SonarSensorCheck start #" + String(sensor + 1), DBG_VERBOSE, true);
  if (sensor == 0)
  {
    DisplayClear();
    DisplayPrint(0, 0, F("Sonar Tests"));
  }

//  Distance = sonar[sensor].ping_cm(SONAR_MAX_DISTANCE);
  Distance = sonar[sensor].convert_cm(sonar[sensor].ping_median(SONAR_READ_ITERATIONS));
  sensorCheck = Distance != 0;

  DebugPrintln(g_sensorStr[sensor] + " Distance " + String(Distance), DBG_INFO, true);

  if (sensorCheck)
  {
    DebugPrintln(g_sensorStr[sensor] + " Sonar sensor Ok : " + String(Distance), DBG_INFO, true);
    DisplayPrint(2, sensor + 1, g_sensorStr[sensor]);
    DisplayPrint(8, sensor + 1, "OK " + String(Distance) + " cm");
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(g_sensorStr[sensor] + " No sensor echo", TAG_CHECK, DBG_WARNING);
    DisplayPrint(2, sensor + 1, g_sensorStr[sensor]);
    DisplayPrint(8, sensor + 1, F("NO ECHO"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to read distance
 * @param sensor int functional sensor to read distance from
 * @param Now optional bool true if immediate read
 * @return float sensor distance
 */
int SonarRead(const int sensor, const bool Now)
{
  static unsigned long LastSonarRead[SONAR_COUNT] = {0, 0, 0};

  if ((millis() - LastSonarRead[sensor] > SONAR_READ_INTERVAL) || Now)
  {
    unsigned int Distance = UNKNOWN_INT;
//    Distance = sonar[sensor].ping_cm(SONAR_MAX_DISTANCE);
    Distance = sonar[sensor].convert_cm(sonar[sensor].ping_median(SONAR_READ_ITERATIONS));


    DebugPrintln("Sonar "+ g_sensorStr[sensor] + " value: " + String(Distance) + " cm", DBG_VERBOSE, true);

    LastSonarRead[sensor] = millis();
    if (Distance == 0)
    {
      g_SonarDistance[sensor] = SONAR_MAX_DISTANCE;
    }
    else
    {
      g_SonarDistance[sensor] = Distance;
    }
  }
  return g_SonarDistance[sensor];
}
