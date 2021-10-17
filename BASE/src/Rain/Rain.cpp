#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Rain/Rain.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
//#include "AnaReadTsk/AnaReadTsk.h"

/**
 * Checks to see if rain sensor is connected (and hopefully functionning)
 * 
 */
bool RainSensorCheck(void)
{
//  int raw = ProtectedAnalogRead(PIN_ESP_RAIN);
  int raw = analogRead(PIN_ESP_RAIN);

  DebugPrintln("Raw Rain value: " + String(raw), DBG_DEBUG, true);

  DisplayClear();
  DisplayPrint(0, 0, F("Rain Test"));

  if (raw >= RAIN_SENSOR_CHECK_THRESHOLD)
  {
    DebugPrintln("Rain Sensor Ok", DBG_INFO, true);
    DisplayPrint(2, 2, F("Rain OK"));
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
    DisplayPrint(2, 2, F("Rain ERROR"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to know if it is raining
 * 
 * @param Now optional bool to force immediate rain sensor read
 * @return true if rain is detected
 */
bool isRaining(const bool Now)
{
  static unsigned long LastRainRead = 0;
  int raw = 0;
  static float smoothValue = UNKNOWN_FLOAT;

  if ((millis() - LastRainRead > RAIN_READ_INTERVAL) || Now)
  {
    // raw = ProtectedAnalogRead(PIN_ESP_RAIN);
    raw = map(analogRead(PIN_ESP_RAIN), 0, 4096, 0, 10000);

    if (smoothValue == UNKNOWN_FLOAT)
    {
      smoothValue = raw / 100.0f;
    }
    else
    {
      smoothValue = 0.80 * smoothValue + 0.20 * ((float)raw / 100.0f);
    }
    g_RainLevel = smoothValue;
    DebugPrintln("Raining check value: " + String(smoothValue), DBG_VERBOSE, true);
    LastRainRead  = millis();
  }
  
  g_IsRainning = smoothValue > RAIN_SENSOR_RAINING_THRESHOLD;

  return g_IsRainning ;
}
