#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "BaseStates/BaseStates.h"
#include "BaseDisplay/BaseDisplay.h"
#include "Rain/Rain.h"
#include "Fan/Fan.h"
#include "PerimeterLoad/PerimeterLoad.h"
#include "PerimeterSendTsk/PerimeterSendTsk.h"
#include "PwrSupply/PwrSupply.h"
#include "Temperature/Temperature.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "MQTT/MQTT.h"
#include "EEPROM/EEPROM.h"

//---------------------------------------------------------------------------------------------------------------------------
//
// Idle Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Base in idle state
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseIdle(const bool StateChange, const BaseState PreviousState)
{
  if (StateChange)
  {
    DebugPrintln("");
    LogPrintln("Base Idle", TAG_STATES, DBG_INFO);

    // Change display with refresh
    idleDisplay(true);

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

    // Reset mower error code (not needed after error acknowledgement implemented)
    g_CurrentErrorCode = ERROR_NO_ERROR;
   }

  //--------------------------------
  // Update raining status and send 
  //--------------------------------
  isRaining();
  BaseRainStatusSend();

  //--------------------------------
  // Update Perimeter status and send 
  //--------------------------------
  PerimeterSignalStatusSend();

  FanCheck(FAN_1_RED);  // Read temperature and activate or stop fan

  PwrSupplyVoltageRead();

  PerimeterLoadCurrentRead();

  // Update display
  idleDisplay();

//   // If Mower is not in Docked mode we can switch to sending mode
//   if (g_BatteryChargeCurrent > MOWER_AT_BASE_CURRENT)
//   {
//     LogPrintln("Mower on charge", TAG_STATES, DBG_INFO);
//     g_CurrentState = MowerState::docked;
//   }

}

//---------------------------------------------------------------------------------------------------------------------------
//
// Sleeping Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Base sleeping
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseSleeping(const bool StateChange, const BaseState PreviousState)
{
  if (StateChange)
  {
    LogPrintln("Base sleeping", TAG_STATES, DBG_INFO);

    // Stop sending
    PerimeterSignalStop();

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL_SLOW;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

    // Change display with refresh
    sleepingDisplay(true);

    EEPROMSave(true); // Update EEPROM
  }

  //--------------------------------
  // Update raining status and send 
  //--------------------------------
  isRaining();
  BaseRainStatusSend();

  //--------------------------------
  // Update Perimeter status and send 
  //--------------------------------
  PerimeterSignalStatusSend();

  PwrSupplyVoltageRead();

  PerimeterLoadCurrentRead();

  // Update display 
  sleepingDisplay();
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Sending Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Base sending perimeter signal mode
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseSending(const bool StateChange, const BaseState PreviousState)
{
  static unsigned long sendingStartTime = 0;

  //--------------------------------
  // Actions to take when entering the state
  //--------------------------------

  if (StateChange)
  {
    DebugPrintln("");
    LogPrintln("Base sending Started", TAG_BASE, DBG_INFO);

    // Change display with refresh
    sendingDisplay(true);

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL_FAST;

    // Reset mower error code (not needed after error acknowledgement implemented)
    g_CurrentErrorCode = ERROR_NO_ERROR;

    //--------------------------------
    // Activate Signal sending
    //--------------------------------
    PerimeterSignalStart();

    //--------------------------------
    // Wait and force read Perimeter current value
    //--------------------------------
    delay(500);
    PerimeterLoadCurrentRead(true);

    //Initialise sending start time
    sendingStartTime = millis();

    // Refresh display
    sendingDisplay(true);

    EEPROMSave(true); // Update EEPROM
  }

  //--------------------------------
  // Sending loop starts here
  //--------------------------------

  // Ongoing Sending routine is as follows:
  //    Check for perimeter current,
  //    Check for temperautre,
  //    Check for rain

  PwrSupplyVoltageRead();

  PerimeterLoadCurrentRead();

  //--------------------------------
  // Check if Perimeter current is ok
  //--------------------------------
  if (PerimeterCurrentTooLowCheck((float(BASE_PERIMETER_CURRENT_TOO_LOW_THRESHOLD) * g_PerimeterPowerLevel) / 100.0f))
  {
    g_totalBaseOnTime = g_totalBaseOnTime + (millis() - sendingStartTime);   // in minutes
    g_BaseCurrentState = BaseState::error;
    g_CurrentErrorCode = ERROR_PERIMETER_CURRENT_TOO_LOW;
    return;
  }

  if (PerimeterCurrentTooHighCheck(BASE_PERIMETER_CURRENT_TOO_HIGH_THRESHOLD))
  {
    g_totalBaseOnTime = g_totalBaseOnTime + (millis() - sendingStartTime);   // in minutes
    g_BaseCurrentState = BaseState::error;
    g_CurrentErrorCode = ERROR_PERIMETER_CURRENT_TOO_HIGH;
    return;
  }

  //--------------------------------
  // Update Fan
  //--------------------------------
  FanCheck(FAN_1_RED);  // Read temperature and activate or stop fan

  //--------------------------------
  // Check temperature too high
  //--------------------------------
  if (BaseTemperatureTooHighCheck(BASE_TEMPERATURE_TOO_HIGH_THRESHOLD))
  {
    g_totalBaseOnTime = g_totalBaseOnTime + (millis() - sendingStartTime);   // in minutes
    g_BaseCurrentState = BaseState::error;
    g_CurrentErrorCode = ERROR_TEMPERATURE_TOO_HIGH;
    return;
  }

  //--------------------------------
  // Update raining status and send 
  //--------------------------------
  isRaining();
  BaseRainStatusSend();

  //--------------------------------
  // Update Perimeter status and send 
  //--------------------------------
  PerimeterSignalStatusSend();

  g_totalBaseOnTime = g_totalBaseOnTime + (millis() - sendingStartTime);   // in minutes
  sendingStartTime = millis();

  // Update display
  sendingDisplay();
}

//---------------------------------------------------------------------------------------------------------------------------
//
// ERROR Mode
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Base in error
 * @param StateChange boolean indicating this is the first call to this state after a state change
 * @param PreviousState BaseState indicating previous state
 */
void BaseInError(const bool StateChange, const BaseState PreviousState)
{
   if (StateChange)
   {
     // STOP Sending
    //  g_enableSender = false;

    // Change display with refresh
    errorDisplay(true);

    // Change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

    DebugPrintln("");
    LogPrintln("Base stopped on Error #" + String(g_CurrentErrorCode) + "-" + ErrorString(g_CurrentErrorCode), TAG_ERROR, DBG_ERROR);

    DebugPrintln("Base Acknowledgement required!", DBG_ERROR, true);
  }
  else
  {
    // Update display
    errorDisplay();

    FanCheck(FAN_1_RED);  // Read temperature and activate or stop fan

  // Update Voltage and current

    PwrSupplyVoltageRead();

    PerimeterLoadCurrentRead();

    // wait for user action (keypad action)
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Helper functions
//
//---------------------------------------------------------------------------------------------------------------------------

/**
 * Base temperature monitoring : Base is set in error mode and sending stopped if temperature above max threshold
 * @param Theshold temperature above which base station is stopped
 * @return boolean indicating if temperature is too high (true) or not (false)
 */
bool BaseTemperatureTooHighCheck(const float Threshold)
{
  if (TemperatureRead(TEMPERATURE_1_RED) > Threshold)
  {
    DebugPrintln("Temperature too high : Perimeter Signal stop (" + String(g_Temperature[TEMPERATURE_1_RED], 1) + " deg)", DBG_ERROR, true);
    PerimeterSignalStop();
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Base Perimeter current monitoring : Base is set in error mode and sending stopped if perimeter current too low
 * @param Threshold current (in mA) under which base station is stopped
 * @return boolean indicating if current is too low (true) or not (false)
 */
bool PerimeterCurrentTooLowCheck(const float Threshold)
{
  if (g_PerimeterCurrent < Threshold)
  {
    DebugPrintln("Perimeter Current too LOW : Perimeter Signal stop (" + String(g_PerimeterCurrent, 0) + " < " + String(Threshold, 1) + " mA)", DBG_ERROR, true);
    PerimeterSignalStop();
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Base Perimeter current monitoring : Base is set in error mode and sending stopped if perimeter current too high
 * @param Threshold current (in mA) over which base station is stopped
 * @return boolean indicating if current is too high (true) or not (false)
 */
bool PerimeterCurrentTooHighCheck(const float Threshold)
{
  if (g_PerimeterCurrent > Threshold)
  {
    DebugPrintln("Perimeter Current too HIGH : Perimeter Signal stop (" + String(g_PerimeterCurrent, 0) + " > " + String(Threshold, 1) + " mA)", DBG_ERROR, true);
    PerimeterSignalStop();
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * Mower status received monitoring : Base is set in error mode if no data is received before timeout and base is set into Sleeping mode if mower is in Docked Mode
 * @param Timeout in ms duing which mower data is expected
 * @return boolean indicating if timeout has been reached (true) or not (false)
 */
bool MowerStatusCheck(const unsigned long Timeout)
{

  // If no mower data is received for too long, reset stored Battery SOC and Charge current
  if (millis() - g_LastMowerTelemetryReceived > Timeout) 
  {
    g_MowerChargeCurrent = UNKNOWN_FLOAT;
    g_MowerBatterySOC = UNKNOWN_FLOAT;

    // If base is not sleeping, set base to error mode
    if (g_CurrentErrorCode != ERROR_NO_MOWER_DATA && 
        g_BaseCurrentState != BaseState::sleeping)
    {
      DebugPrintln("No mower data received for " + String(Timeout/1000) + " seconds : Perimeter Signal stop ", DBG_ERROR, true);
      PerimeterSignalStop();
      g_BaseCurrentState = BaseState::error;
      g_CurrentErrorCode = ERROR_NO_MOWER_DATA;
      return true;
    }
  }

  // If mower is docked and base is not sleeping, set base to sleeping mode
  if (g_MowerCurrentState == MowerState::docked && g_BaseCurrentState != BaseState::sleeping && millis() - g_LastMowerTelemetryReceived < Timeout)
  {
    DebugPrintln ("Mower State is docked, switching base to sleeping", DBG_INFO, true);
    g_BaseCurrentState = BaseState::sleeping;
    g_CurrentErrorCode = ERROR_NO_ERROR;
  }
  return false;
}
