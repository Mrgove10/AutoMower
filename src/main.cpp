#include <Arduino.h>
#include "myGlobals_definition.h"
#include "states.h"
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

MowerState CurrentState = MowerState::test;

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
  CurrentState = MowerState::mowing;
}

void MOWERERROR()
{
  // STOP all motors
  // disable sensors
  // send notification to phone
  // send telemetry
}

void stopAllWheel(){
  MotionMotorStop(MOTION_MOTOR_RIGHT);
  MotionMotorStop(MOTION_MOTOR_LEFT);
}

void loop()
{

  EEPROMSave(false);

  switch (CurrentState)
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
}