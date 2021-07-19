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

void setup()
{
  MySetup();
}

void IDLE()
{
  // Waiting for input ?
  // Send telemetry
}

void DOCKED()
{
  // wait for GO
  // send telemetry
  // if battery is egnoth
  // GO
  // else
  // send error bat low
}

void turn(int angle, bool direction)
{
  // TODO : make an enum here
  // stop motor
  // one moto go forward
  // one motor go back
  // change dir
  // go front
}

void uTurn()
{
  // TODO
  turn(180, true);
}

void getMeUnstuck()
{
  // stop motor
  // go back 10 cm
  // turn right or left (by certain angle)
  turn(15, true);
  // go forward
}

void MOWING()
{
  // cutblade 100%
  if (g_RightBumperTriggered || g_LeftBumperTriggered)
  {
    uTurn();
  }
  // if sonar
  // if wire
}

void GOTOBASE()
{
  // stop motors
  // find target with compas
  // go forward
  // detect perim
  // folow perim to base
}

void LEAVEBASE()
{
  // go backward for 50 cm
  uTurn();
  //go forward
  g_CurrentState = MowerState::mowing;
}

void MOWERERROR()
{
  // STOP all motors
  // disable sensors
  // send notification to phone
  // send telemetry
}

void loop()
{
  // Common routine mower tasks

  FanCheck(FAN_1_RED);        // Read temperature and activate or stop Cutting fan
  FanCheck(FAN_2_BLUE);       // Read temperature and activate or stop Motion fan

  switch (g_CurrentState)
  {
  case MowerState::idle:
    IDLE();
    break;

  case MowerState::docked:
    DOCKED();
    break;

  case MowerState::mowing:
    MOWING();
    break;

  case MowerState::going_to_base:
    GOTOBASE();
    break;

  case MowerState::leaving_base:
    LEAVEBASE();
    break;

  case MowerState::error:
    MOWERERROR();
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

  EEPROMSave(false);        // Update EEPROM

  MQTTReconnect();          // Check MQTT Status ans reconnect

  MQTTSendTelemetry();      // Send Mower Telemetry

  MQTTclient.loop();        // Update MQTT

  SerialAndTelnet.handle(); // Refresh Telnet Session

  events();                 // eztime refresh

  delay(50);
}