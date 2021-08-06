#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Rain/Rain.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "FastAnaReadTsk/FastAnaReadTsk.h"

/**
 * Checks to see if rain sensor is connected (and hopefully functionning)
 * 
 */
bool RainSensorCheck(void)
{
  int raw = ProtectedAnalogRead(PIN_ESP_RAIN);

  DebugPrintln("Raw Rain value: " + String(raw), DBG_VERBOSE, true);

  DisplayClear();
  DisplayPrint(0 , 0, F("Rain Test"));

  if (raw > RAIN_SENSOR_CHECK_THRESHOLD)
  {
    DebugPrintln("Rain Sensor Ok", DBG_INFO, true);
    DisplayPrint(2 , 2, F("Rain OK"));
    if (raw > RAIN_SENSOR_RAINING_THRESHOLD)
    {
      DisplayPrint(9, 2, F(" + rain"));
    }
    delay(TEST_SEQ_STEP_WAIT);

    return true;
  }
  else
  {
    LogPrintln("Rain Sensor not found", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2 , 2, F("Rain ERROR"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to know if it is raining
 * 
 * * @return true if rain is detected
 */
bool isRaining(void)
{
  int raw = ProtectedAnalogRead(PIN_ESP_RAIN);
  DebugPrintln("Raining check value: " + String(raw), DBG_VERBOSE, true);

  return raw > RAIN_SENSOR_RAINING_THRESHOLD;
}
