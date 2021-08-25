#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Battery/Battery.h"
#include "Current/Current.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "IOExtender/IOExtender.h"

/**
 * Battery charge relay setup function
 * 
 */
void BatteryChargeRelaySetup(void)
{
  DebugPrintln("Battery charge Relay setup", DBG_VERBOSE, true);

  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  // Set pin mode
   IOExtend.pinMode(PIN_MCP_BATTERY_CHARGE_RELAY, OUTPUT_OPEN_DRAIN);
  // IOExtend.pinMode(PIN_MCP_BATTERY_CHARGE_RELAY, OUTPUT);

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  // On start, close relay (relay is Normaly Open)
  BatteryChargeRelayClose();
  delay(5000);
  BatteryChargeRelayOpen();
  delay(5000);
  BatteryChargeRelayClose();
  delay(5000);
  BatteryChargeRelayOpen();
  delay(5000);
  BatteryChargeRelayClose();
}

/**
 * Battery charge relay Open function
 * 
 */
void BatteryChargeRelayOpen(void)
{
  if (g_BatteryRelayIsClosed)
  {
    // Open relay (relay is Normaly Open)
    IOExtendProtectedWrite(PIN_MCP_BATTERY_CHARGE_RELAY, HIGH);

    g_BatteryRelayIsClosed = false;

    DebugPrintln("Battery charge relay opened", DBG_VERBOSE, true);
  }
}

/**
 * Battery charge relay Close function
 * 
 */
void BatteryChargeRelayClose(void)
{
  if (!g_BatteryRelayIsClosed)
  {
    // Close relay (relay is Normaly Open)
    IOExtendProtectedWrite(PIN_MCP_BATTERY_CHARGE_RELAY, LOW);

    g_BatteryRelayIsClosed = true;

    DebugPrintln("Battery charge relay closed", DBG_VERBOSE, true);
  }
}

/**
 * Battery charge check function. If battery is full, charge relay is opened. If battery level drops under a threshold, relay is closed
 * 
 * @param Now optional bool to force immediate battery check
 */
void BatteryChargeCheck(const bool Now)
{
  static unsigned long LastBatteryCheck = 0;

  if ((millis() - LastBatteryCheck > BATTERY_CHECK_INTERVAL) || Now)
  {
    BatteryChargeCurrentRead(Now);

    // Check if battery Charge current and voltage levels correspond to a fully charged battery
    if (g_BatteryChargeCurrent < BATTERY_CHARGE_CURRENT_TO_STOP_CHARGE && g_BatteryVoltage > BATTERY_VOLTAGE_TO_STOP_CHARGE && g_BatteryIsCharging)
    {
      // Open relay to stop charge
      BatteryChargeRelayOpen();
      DebugPrintln("Battery Full, charge stopped (" + String(g_BatteryChargeCurrent) + " mA, " + String(g_BatteryVoltage) + " mV)" , DBG_INFO, true);
    }

    // Check if battery voltage level is below charging threshold
    if (g_BatteryVoltage < BATTERY_VOLTAGE_TO_START_CHARGE)
    {
      DebugPrintln("Battery needs charge (" + String(g_BatteryVoltage) + " mV)" , DBG_INFO, true);
      // Close relay to enable charge
      BatteryChargeRelayClose();
    }

    // Determine charging status
    if (g_BatteryRelayIsClosed && g_BatteryChargeCurrent > BATTERY_CHARGE_CURRENT_CHARGING_THRESHOLD)
    {
        g_BatteryIsCharging = true;
    }
    else
    {
        g_BatteryIsCharging = false;
    }

    // Determine State of Charge

    g_BatterySOC = (map(int(g_BatteryVoltage*10), int(BATTERY_0_PERCENT_VOLTAGE*10), int(BATTERY_VOLTAGE_FULL_THRESHOLD*10), 0, 1000))/10;
    
    if (g_BatteryVoltage <= BATTERY_0_PERCENT_VOLTAGE)
    { 
      g_BatterySOC = 0;
    }

    if (g_BatteryVoltage >= BATTERY_VOLTAGE_FULL_THRESHOLD)
    { 
      g_BatterySOC = 100;
    }

    LastBatteryCheck = millis();
  }
}
