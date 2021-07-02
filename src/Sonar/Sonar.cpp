#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Sonar/Sonar.h"
#include "Utils/Utils.h"

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
  String sensorStr[SONAR_COUNT] = {"Front", "Left","Right" } ;
  unsigned int Distance = UNKNOWN_INT;

  DebugPrintln("SonarSensorCheck start #" + String(sensor + 1), DBG_VERBOSE, true);
  if (sensor == 0) 
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Sonar Tests"));
  }

  Distance = sonar[sensor].ping_cm(SONAR_MAX_DISTANCE);
  sensorCheck = Distance != 0;

  lcd.setCursor(2, sensor + 1);

  DebugPrintln(sensorStr[sensor] + " Distance " + String(Distance), DBG_INFO, true);

  if (sensorCheck) 
  {
    DebugPrintln(sensorStr[sensor] + " Sonar sensor Ok : " + String(Distance), DBG_INFO, true);
    lcd.print(sensorStr[sensor]);
    lcd.setCursor(8, sensor + 1);
    lcd.print(F("OK "));
    lcd.print(Distance);
    lcd.print(F(" cm"));
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(sensorStr[sensor] + " No sensor echo", TAG_CHECK, DBG_WARNING);
    lcd.print(sensorStr[sensor]);
    lcd.setCursor(8, sensor + 1);
    lcd.print(F("NO ECHO"));
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
    Distance = sonar[sensor].ping_cm(SONAR_MAX_DISTANCE);

//    DebugPrintln("TemperatureRead value " + String(tempC,2), DBG_VERBOSE, true);

    LastSonarRead[sensor] = millis();
    SonarDistance[sensor] = Distance;
    return Distance;
  }
  return SonarDistance[sensor];
}
