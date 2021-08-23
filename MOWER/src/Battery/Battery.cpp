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

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  // On start, close relay (relay is Normaly Open)
  BatteryChargeRelayClose();
}

/**
 * Battery charge relay Open function
 * 
 */
void BatteryChargeRelayOpen(void)
{
  // Open relay (relay is Normaly Open)
  IOExtendProtectedWrite(PIN_MCP_BATTERY_CHARGE_RELAY, LOW);

  g_BatteryRelayIsClosed = false;

  DebugPrintln("Battery charge relay opened", DBG_VERBOSE, true);
}

/**
 * Battery charge relay Close function
 * 
 */
void BatteryChargeRelayClose(void)
{
  // Close relay (relay is Normaly Open)
  IOExtendProtectedWrite(PIN_MCP_BATTERY_CHARGE_RELAY, HIGH);

  g_BatteryRelayIsClosed = true;

  DebugPrintln("Battery charge relay closed", DBG_VERBOSE, true);
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
    if (g_BatteryChargeCurrent < BATTERY_CHARGE_CURRENT_TO_STOP_CHARGE && g_BatteryVotlage > BATTERY_VOLTAGE_TO_STOP_CHARGE)
    {
      // Open relay to stop charge
      BatteryChargeRelayOpen();
      DebugPrintln("Battery Full, charge stopped", DBG_INFO, true);
    }

    // Check if battery voltage level is below charging threshold
    if (g_BatteryVotlage < BATTERY_VOLTAGE_TO_START_CHARGE)
    {
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
    g_BatterySOC = (map(int(g_BatteryVotlage*10), int(BATTERY_0_PERCENT_VOLTAGE*10), int(BATTERY_100_PERCENT_VOLTAGE*10), 0, 1000))/10;
    
    LastBatteryCheck = millis();
  }
}
