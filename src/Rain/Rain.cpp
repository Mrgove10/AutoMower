#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "Rain/Rain.h"
#include "Utils/Utils.h"
/**
 * Checks to see if rain sensor is connected (and hopefully functionning)
 * 
 */
bool RainSensorCheck(void)
{
  int raw = analogRead(PIN_ESP_RAIN);
  DebugPrintln("Raw Rain value: " + String(raw), DBG_VERBOSE, true);
  
  if (raw > RAIN_SENSOR_CHECK_THRESHOLD) {
      DebugPrintln("Rain Sensor Ok", DBG_INFO, true);
      return true;
  }
  else
  {
      LogPrintln("Rain Sensor not found", TAG_CHECK, DBG_ERROR);
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
  int raw = analogRead(PIN_ESP_RAIN);
  DebugPrintln("Raining check value: " + String(raw), DBG_VERBOSE, true);
  
  return raw > RAIN_SENSOR_RAINING_THRESHOLD;
}
