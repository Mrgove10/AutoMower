#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Rain/Rain.h"
#include "Utils/Utils.h"
#include "LCD/LCD.h"

/**
 * Checks to see if rain sensor is connected (and hopefully functionning)
 * 
 */
bool RainSensorCheck(void)
{
  int raw = analogRead(PIN_ESP_RAIN);

  DebugPrintln("Raw Rain value: " + String(raw), DBG_VERBOSE, true);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Rain Test"));
  lcd.setCursor(2, 2);

  if (raw > RAIN_SENSOR_CHECK_THRESHOLD)
  {
    DebugPrintln("Rain Sensor Ok", DBG_INFO, true);
    lcd.print(F("Rain OK"));
    if (raw > RAIN_SENSOR_RAINING_THRESHOLD)
    {
      lcd.print(F(" + rain"));
    }
    delay(TEST_SEQ_STEP_WAIT);

    return true;
  }
  else
  {
    LogPrintln("Rain Sensor not found", TAG_CHECK, DBG_ERROR);
    lcd.print(F("Rain ERROR"));
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
  int raw = analogRead(PIN_ESP_RAIN);
  DebugPrintln("Raining check value: " + String(raw), DBG_VERBOSE, true);
  
  return raw > RAIN_SENSOR_RAINING_THRESHOLD;
}
