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

void TestLoop()
{
  static int MotionSpeed = 0;
  static int MotionSens = 1;
  static int MotionDirection = MOTION_MOTOR_STOPPED;
  static int CutSpeed = 0;
  static int CutSens = 1;
  static int CutDirection = MOTION_MOTOR_STOPPED;

  if (g_RightBumperTriggered)
  {
    DebugPrintln("Right Bumper Triggered !", DBG_INFO, true);
    g_RightBumperTriggered = false;
  }
  if (g_LeftBumperTriggered)
  {
    DebugPrintln("Left Bumper Triggered !", DBG_INFO, true);
    g_LeftBumperTriggered = false;
  }

  if (g_HorizontalTiltTriggered)
  {
    DebugPrintln("Horizontal Tilt sensor Triggered !", DBG_INFO, true);
    g_HorizontalTiltTriggered = false;
  }

  if (g_VerticalTiltTriggered)
  {
    DebugPrintln("Vertical Tilt sensor Triggered !", DBG_INFO, true);
    g_VerticalTiltTriggered = false;
  }

  BatteryChargeCurrentRead(false);
  MotorCurrentRead(MOTOR_CURRENT_RIGHT);
  MotorCurrentRead(MOTOR_CURRENT_LEFT);
  MotorCurrentRead(MOTOR_CURRENT_CUT);
  
  KeypadRead();

  //  TemperatureRead(TEMPERATURE_1_RED);   // not needed : Done by FanCheck()
  //  TemperatureRead(TEMPERATURE_2_BLUE);   // not needed : Done by FanCheck()

  SonarRead(SONAR_FRONT);
  SonarRead(SONAR_LEFT);
  SonarRead(SONAR_RIGHT);

  BatteryVoltageRead();

  CompassRead();

  GPSRead(true);

  CutMotorCheck();

  static unsigned long LastRefresh = 0;

  if ((millis() - LastRefresh > 2000))
  {

// Motion motor loop
    MotionSpeed = MotionSpeed + (1 * MotionSens);
    if (MotionSpeed > 120)
    {
      MotionSens = -1;
    }
    if (MotionSpeed < -120)
    {
      MotionSens = 1;
    }
    if (MotionSpeed < 0)
    {
      if (MotionDirection != MOTION_MOTOR_REVERSE)
      {
        MotionDirection = MOTION_MOTOR_REVERSE;
        MotionMotorStop(MOTION_MOTOR_RIGHT);
        MotionMotorStop(MOTION_MOTOR_LEFT);
      }
    }
    else
    {
      if (MotionDirection != MOTION_MOTOR_FORWARD)
      {
        MotionDirection = MOTION_MOTOR_FORWARD;
        MotionMotorStop(MOTION_MOTOR_RIGHT);
        MotionMotorStop(MOTION_MOTOR_LEFT);
      }
    }
    if (!g_MotionMotorOn[MOTION_MOTOR_RIGHT])
    {
      MotionMotorStart(MOTION_MOTOR_RIGHT, MotionDirection, abs(MotionSpeed));
      MotionMotorStart(MOTION_MOTOR_LEFT, MotionDirection, abs(MotionSpeed));
    }
    else
    {
      MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, abs(MotionSpeed));
      MotionMotorSetSpeed(MOTION_MOTOR_LEFT, abs(MotionSpeed));
    }
  
// Cut motor loop
    CutSpeed = CutSpeed + (24 * CutSens);
    if (CutSpeed > 4096 + 1024)
    {
      CutSens = -1;
    }
    if (CutSpeed < -4096 - 1024)
    {
      CutSens = 1;
    }
    if (CutSpeed <= 0)
    {
      if (CutDirection != CUT_MOTOR_REVERSE)
      {
        CutDirection = CUT_MOTOR_REVERSE;
        CutMotorStop(true);
      }
    }
    else
    {
      if (CutDirection != CUT_MOTOR_FORWARD)
      {
        CutDirection = CUT_MOTOR_FORWARD;
        CutMotorStop(true);
      }
    }
    if (!g_CutMotorOn)
    {
      CutMotorStart(CutDirection, abs(CutSpeed));
    }
    else
    {
      CutMotorSetSpeed(abs(CutSpeed));
    }
  LastRefresh = millis();
  }
  SerialAndTelnet.handle();
}