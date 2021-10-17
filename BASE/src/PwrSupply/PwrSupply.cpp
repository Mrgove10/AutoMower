#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "PwrSupply/PwrSupply.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "IOExtender/IOExtender.h"

/**
 * Checks to see if voltage is connected and its level
 * 
 * @return true if voltage check is ok
 */
bool PwrSupplyVoltageCheck(void)
{
  String StatusStr[4] = {"Ok", "Medium", "Low", "CRITICAL"};

  int status = PwrSupplyVoltageRead();

  DebugPrintln("Power Supply status: " + StatusStr[status], DBG_INFO, true);

  DisplayClear();
  DisplayPrint(0, 0, F("Supply Test"));
  DisplayPrint(2, 2, "Supply " + StatusStr[status]);
  DisplayPrint(12, 3, String(float(g_PwrSupplyVoltage / 1000.0f), 1) + " V");

  if (status <= PWR_SUPPLY_VOLTAGE_LOW)
  {
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("Power Supply Level low:" + String(float(g_PwrSupplyVoltage / 1000.0f), 1) + " V", TAG_CHECK, DBG_ERROR);
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
int PwrSupplyVoltageRead(const bool Now)
{
  static unsigned long LastVoltageRead = 0;
  static float smoothVoltage = UNKNOWN_FLOAT;

  if ((millis() - LastVoltageRead > PWR_SUPPLY_VOLTAGE_READ_INTERVAL) || Now)
  {
    int voltraw = analogRead(PIN_ESP_SUPPLY_VOLTAGE);
    int volt = map(voltraw, 0, 4095, 0, VOLTAGE_RANGE_MAX);

    if (smoothVoltage == UNKNOWN_FLOAT)
    {
     smoothVoltage = volt;
    }
    else
    {
      smoothVoltage = 0.90 * smoothVoltage + 0.10 * ((float)volt);
    }

    g_PwrSupplyVoltage = smoothVoltage;
    LastVoltageRead = millis();

    DebugPrintln("Power Supply Voltage: " + String(g_PwrSupplyVoltage) + " mV", DBG_VERBOSE, true);

    if (g_PwrSupplyVoltage < PWR_SUPPLY_VOLTAGE_LOW_THRESHOLD)
    {
      g_PwrSupplyStatus = PWR_SUPPLY_VOLTAGE_CRITICAL;
      return PWR_SUPPLY_VOLTAGE_CRITICAL;
    }
    else if (g_PwrSupplyVoltage < PWR_SUPPLY_VOLTAGE_MEDIUM_THRESHOLD)
    {
      g_PwrSupplyStatus = PWR_SUPPLY_VOLTAGE_LOW;
      return PWR_SUPPLY_VOLTAGE_LOW;
    }
    else if (g_PwrSupplyVoltage < PWR_SUPPLY_VOLTAGE_NORMAL_THRESHOLD)
    {
      g_PwrSupplyStatus = PWR_SUPPLY_VOLTAGE_MEDIUM;
      return PWR_SUPPLY_VOLTAGE_MEDIUM;
    }
    else
    {
      g_PwrSupplyStatus = PWR_SUPPLY_VOLTAGE_OK;
      return PWR_SUPPLY_VOLTAGE_OK;
    }
  }
  return g_PwrSupplyStatus;
}