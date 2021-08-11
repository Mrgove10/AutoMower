#include <Arduino.h>
#include "myGlobals_definition.h"
#include "mySetup.h"
#include "Utils/Utils.h"
#include "MQTT/MQTT.h"
#include "Temperature/Temperature.h"
#include "Current/Current.h"
#include "Voltage/Voltage.h"
#include "Sonar/Sonar.h"
#include "EEPROM/EEPROM.h"
#include "Keypad/Keypad.h"
#include "Fan/Fan.h"
#include "Compass/Compass.h"
#include "GPS/GPS.h"
#include "IOExtender/IOExtender.h"
#include "MotionMotor/MotionMotor.h"
#include "CutMotor/CutMotor.h"
#include <pin_definitions.h>
#include "Display/Display.h"
#include "TestLoop.h"
#include "DisplayMowerData.h"
#include "MowerStates/MowerStates.h"

void setup()
{
  MySetup();

// Set mower to idle mode
  MowerIdle(true, g_PreviousState);
}

void loop()
{
  static MowerState StateOnCall = MowerState::idle;

  // Common routine mower tasks

  FanCheck(FAN_1_RED);  // Read temperature and activate or stop Cutting fan
  FanCheck(FAN_2_BLUE); // Read temperature and activate or stop Motion fan

  bool stateChange = g_CurrentState != StateOnCall;

  if (stateChange){
      g_PreviousState = g_CurrentState;
  }

  StateOnCall = g_CurrentState;

  switch (g_CurrentState)
  {
  case MowerState::idle:
    MowerIdle(stateChange, g_PreviousState);
    break;

  case MowerState::docked:
    MowerDocked(stateChange, g_PreviousState);
    break;

  case MowerState::mowing:
    MowerMowing(stateChange, g_PreviousState);
    break;

  case MowerState::going_to_base:
    MowerGoingToBase(stateChange, g_PreviousState);
    break;

  case MowerState::leaving_base:
    MowerLeavingBase(stateChange, g_PreviousState);
    break;

  case MowerState::error:
    MowerInError(stateChange, g_PreviousState);
    break;

  case MowerState::test:
    TestLoop();
    break;

  default:
    break;
  }

  // Display Mower Data

  DisplayMowerData();

  // Routine system operating tasks

  EEPROMSave(false); // Update EEPROM

  MQTTReconnect(); // Check MQTT Status ans reconnect

  MQTTSendTelemetry(); // Send Mower Telemetry

  MQTTclient.loop(); // Update MQTT

  SerialAndTelnet.handle(); // Refresh Telnet Session

  events(); // eztime refresh

  delay(50);
}