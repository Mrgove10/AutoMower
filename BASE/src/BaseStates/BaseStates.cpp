#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "BaseStates/BaseStates.h"
#include "BaseDisplay/BaseDisplay.h"
#include "Rain/Rain.h"
#include "Fan/Fan.h"
#include "PerimeterLoad/PerimeterLoad.h"
#include "PwrSupply/PwrSupply.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "MQTT/MQTT.h"

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

//     // if (PreviousState == MowerState::mowing)
//     // {
//     MowerStop();
//     CutMotorStop();
//     // }

   }

  isRaining();

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
    g_enableSender = false;
    // TO DO

    //change Telemetry frequency
    g_MQTTSendInterval = MQTT_TELEMETRY_SEND_INTERVAL_SLOW;

    // Force a Telemetry send
    MQTTSendTelemetry(true);

  // Change display with refresh
    sleepingDisplay(true);
  }

  FanCheck(FAN_1_RED);  // Read temperature and activate or stop fan

  isRaining();

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

    g_enableSender = true;          // activate Sending
//  TO DO

    //Initialise sending start time
    sendingStartTime = millis();

    // Refresh display
    sendingDisplay(true);
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
//  TO DO

  //--------------------------------
  // Update Fan
  //--------------------------------
  FanCheck(FAN_1_RED);  // Read temperature and activate or stop fan

  //--------------------------------
  // Check temperature too high
  //--------------------------------
//  TO DO

//   // if (isRaining())
//   // {
//   //   DebugPrintln("Raining : returning to base", DBG_INFO, true);
//   //   MowerStop();
//   //   CutMotorStop();
//       g_totalMowingTime = g_totalMowingTime + (millis() - mowingStartTime);   // in minutes
//       g_CurrentState = MowerState::error;
//       g_CurrentErrorCode = ERROR_MOWING_CONSECUTIVE_OBSTACLES;
//       return;

//   // }

  //--------------------------------
  // Is it rainning ?
  //--------------------------------
  isRaining();

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
     g_enableSender = false;
//     MowerStop();
//     CutMotorStop(true);

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

    // wait for user action (keypad action)
  }
}

//---------------------------------------------------------------------------------------------------------------------------
//
// Helper functions
//
//---------------------------------------------------------------------------------------------------------------------------

// TO DO 

// Check temperature too high
// Check perimeter current too low or too high
// Send Status (ON/OFF or rain) update over MQTT to Mower