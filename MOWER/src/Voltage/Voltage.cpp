#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Voltage/Voltage.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "AnaReadTsk/AnaReadTsk.h"

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

  DisplayClear();
  DisplayPrint(0, 0, F("Battery Test"));
  DisplayPrint(2, 2, "Battery " + StatusStr[status]);
  DisplayPrint(7, 3, String(float(g_BatteryVoltage / 1000.0f), 1) + " V");

  if (status > BATTERY_VOLTAGE_LOW_THRESHOLD)
  {
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("Battery Level low:" + String(float(g_BatteryVoltage / 1000.0f), 1) + " V", TAG_CHECK, DBG_ERROR);
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to read voltage 
 * 
 * @param Now optional bool to force immediate voltage read
 * @return Range depending on voltage thresholds
 */
int BatteryVoltageRead(const bool Now)
{
  static unsigned long LastVoltageRead = 0;
  static float smoothVoltage = UNKNOWN_FLOAT;
  float busvoltage = 0;

  if ((millis() - LastVoltageRead > BATTERY_VOLTAGE_READ_INTERVAL) || Now)
  {
        // Ensure exlusive access to I2C
    xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

    busvoltage = BatteryChargeSensor.getBusVoltage_V();

    // Free access to I2C for other tasks
    xSemaphoreGive(g_I2CSemaphore);

//    int voltraw = ProtectedAnalogRead(PIN_ESP_BAT_VOLT);
//    int volt = map(voltraw, 0, 4095, 0, VOLTAGE_RANGE_MAX);

    int volt = busvoltage*1000;
    
    if (smoothVoltage == UNKNOWN_FLOAT)
    {
     smoothVoltage = volt;
    }
    else
    {
      smoothVoltage = 0.80 * smoothVoltage + 0.20 * ((float)volt);
    }

    g_BatteryVoltage = smoothVoltage;
    LastVoltageRead = millis();

    if (volt < BATTERY_VOLTAGE_LOW_THRESHOLD)
    {
      g_BatteryStatus = BATTERY_VOLTAGE_CRITICAL;
      return BATTERY_VOLTAGE_CRITICAL;
    }
    else if (volt < BATTERY_VOLTAGE_MEDIUM_THRESHOLD)
    {
      g_BatteryStatus = BATTERY_VOLTAGE_LOW;
      return BATTERY_VOLTAGE_LOW;
    }
    else if (volt < BATTERY_VOLTAGE_NORMAL_THRESHOLD)
    {
      g_BatteryStatus = BATTERY_VOLTAGE_MEDIUM;
      return BATTERY_VOLTAGE_MEDIUM;
    }
    else
    {
      g_BatteryStatus = BATTERY_VOLTAGE_OK;
      return BATTERY_VOLTAGE_OK;
    }
  }
  return g_BatteryStatus;
}