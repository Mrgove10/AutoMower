#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Battery/Battery.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "IOExtender/IOExtender.h"

/**
 * I2C INA219 Battery Charge Current Sensor Setup function
 *
 */
void BatteryCurrentSensorSetup()
{
  if (!BatteryChargeSensor.begin())
  {
    LogPrintln("Battery charge current Sensor not found !", TAG_CHECK, DBG_ERROR);
  }
  else
  {
    DebugPrintln("Battery charge current Sensor found", DBG_VERBOSE, true);
  }

  BatteryChargeSensor.setCalibration_32V_2A();
  delay(100);
  BatteryChargeCurrentRead();
}

/**
 * Checks to see if Battery ACS712 current sensor is connected (and hopefully functionning)
 * 
 * @return true if Battery ACS712 current sensor is ok
 */
bool BatteryCurrentSensorCheck(void)
{
  DisplayClear();
  DisplayPrint(0, 0, F("Charge sensor Test"));

  if (BatteryChargeSensor.success())
  {
    DebugPrintln("Charge Sensor ok, Value: " + String(g_BatteryChargeCurrent, 0), DBG_INFO, true);
    DisplayPrint(1, 2, "Charge OK: " + String(g_BatteryChargeCurrent, 0) + " mA");
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln("Battery charge Sensor not found", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 2, F("Charge ERROR"));
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Function to test and read Battery Charge current
 * 
 * * @param Now true to force an immediate read
 * * @return true if charge current could be read
 */
bool BatteryChargeCurrentRead(const bool Now)
{
  static unsigned long LastBatteryChargeCurrentRead = 0;
  static float smoothedCurrent = UNKNOWN_FLOAT;
  float busvoltage = 0;

  if ((millis() - LastBatteryChargeCurrentRead > BATTERY_CHARGE_READ_INTERVAL) || Now)
  {
    float current_mA = 0;

    // Ensure exlusive access to I2C
    xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

    current_mA = BatteryChargeSensor.getCurrent_mA();
    busvoltage = BatteryChargeSensor.getBusVoltage_V();

    // Free access to I2C for other tasks
    xSemaphoreGive(g_I2CSemaphore);

    if (BatteryChargeSensor.success())
    {
      // Filter out very low current values
      if (current_mA < BATTERY_CHARGE_CURRENT_MIN)
      {
        current_mA = 0;
      }

    // Reset average on first value and on leaving base
      if (smoothedCurrent == UNKNOWN_FLOAT || g_CurrentState == MowerState::leaving_base)
      {
        smoothedCurrent = abs(current_mA);
      }
      else
      {
        smoothedCurrent = 0.5 * smoothedCurrent + 0.5 * ((float)abs(current_mA));
      }

      g_BatteryChargeCurrent = smoothedCurrent;
      LastBatteryChargeCurrentRead = millis();

      DebugPrintln("Battery charge current value:" + String(g_BatteryChargeCurrent) + " mA (INA bus voltage:" + String(busvoltage,3) + " V)", DBG_VERBOSE, true);
      return true;
    }
    else
    {
      g_BatteryChargeCurrent = UNKNOWN_FLOAT;
      DebugPrintln("Battery charge current could not be read !", DBG_VERBOSE, true);
      return false;
    }
  }

  return true;
}

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
  DisplayPrint(12, 3, String(float(g_BatteryVoltage / 1000.0f), 1) + " V");

  if (status <= BATTERY_VOLTAGE_LOW)
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
      smoothVoltage = 0.90 * smoothVoltage + 0.10 * ((float)volt);
    }

    g_BatteryVoltage = smoothVoltage;
    LastVoltageRead = millis();

    DebugPrintln("Battery Voltage: " + String(g_BatteryVoltage) + " mV", DBG_VERBOSE, true);

    if (g_BatteryVoltage < BATTERY_VOLTAGE_LOW_THRESHOLD)
    {
      g_BatteryStatus = BATTERY_VOLTAGE_CRITICAL;
      return BATTERY_VOLTAGE_CRITICAL;
    }
    else if (g_BatteryVoltage < BATTERY_VOLTAGE_MEDIUM_THRESHOLD)
    {
      g_BatteryStatus = BATTERY_VOLTAGE_LOW;
      return BATTERY_VOLTAGE_LOW;
    }
    else if (g_BatteryVoltage < BATTERY_VOLTAGE_NORMAL_THRESHOLD)
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
  // delay(5000);
  // BatteryChargeRelayOpen();
  // delay(5000);
  // BatteryChargeRelayClose();
  // delay(5000);
  // BatteryChargeRelayOpen();
  // delay(5000);
  // BatteryChargeRelayClose();
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

    g_BatterySOC = (map(int(g_BatteryVoltage), int(BATTERY_0_PERCENT_VOLTAGE), int(BATTERY_VOLTAGE_FULL_THRESHOLD), 0, 100));
    
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
