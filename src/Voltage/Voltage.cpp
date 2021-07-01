#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Voltage/Voltage.h"
#include "Utils/Utils.h"
#include "LCD/LCD.h"

/**
 * Checks to see if voltage is connected and its level
 * 
 * @return true if voltage check is ok
 */
bool BatteryVoltageCheck(void)
{
  String StatusStr[4] = {"Ok", "Medium", "Low", "CRITICAL"};

  int status = BatteryVoltageRead();

  DebugPrintln("Battery Voltage status: " + StatusStr[status], DBG_INFO, true);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Battery Test"));
  lcd.setCursor(2, 2);
  lcd.print (F("Battery "));
  lcd.print (StatusStr[status]);
  lcd.print (" " + String(float(BatteryVotlage/1000.0f),1) + " V");

  if (status > BATTERY_VOLTAGE_LOW_THRESHOLD)  
  {
     delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("Battery Level low", TAG_CHECK, DBG_ERROR);
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}


/**
 * Function to read voltage 
 * 
 * * @return Range depending on voltage thresholds
 */
int BatteryVoltageRead(void)
{
  int voltraw = analogRead(PIN_ESP_BAT_VOLT);
  int volt = map(voltraw, 0, 4095, 0, VOLTAGE_RANGE_MAX);

  BatteryVotlage = volt;

  if (volt < BATTERY_VOLTAGE_LOW_THRESHOLD) 
  {
    return BATTERY_VOLTAGE_CRITICAL;
  }
  else if (volt < BATTERY_VOLTAGE_MEDIUM_THRESHOLD)
  {
    return BATTERY_VOLTAGE_LOW;
  }
  else if (volt < VOLTAGE_NORMAL_THRESHOLD)
  {
    return BATTERY_VOLTAGE_MEDIUM;
  }
  else 
  {
    return BATTERY_VOLTAGE_OK;
  }
}