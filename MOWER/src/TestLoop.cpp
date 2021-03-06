#include <Arduino.h>
#include "myGlobals_definition.h"
#include "mySetup.h"
#include "Utils/Utils.h"
#include "MQTT/MQTT.h"
#include "Temperature/Temperature.h"
#include "MotorCurrent/MotorCurrent.h"
#include "Battery/Battery.h"
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
#include "MowerDisplay/MowerDisplay.h"

void TestLoop()
{
  static int MotionSpeed = 0;
  static int MotionSens = 1;
  static int MotionDirection = MOTION_MOTOR_STOPPED;
  static int CutSpeed = 0;
  static int CutSens = 1;
  static int CutDirection = MOTION_MOTOR_STOPPED;

  BatteryChargeCurrentRead(false);
  MotorCurrentRead(MOTOR_CURRENT_RIGHT);
  MotorCurrentRead(MOTOR_CURRENT_LEFT);
  MotorCurrentRead(MOTOR_CURRENT_CUT);

  KeypadRead();
  testDisplay(true);

  //  TemperatureRead(TEMPERATURE_1_RED);   // not needed : Done by FanCheck()
  //  TemperatureRead(TEMPERATURE_2_BLUE);   // not needed : Done by FanCheck()

  //  SonarRead(SONAR_RIGHT, true);
  //  SonarRead(SONAR_FRONT, true);
  //  SonarRead(SONAR_LEFT, true);

  // BatteryVoltageRead();

  // CompassRead();

  // GPSRead(true);

  // CutMotorCheck();

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
    CutSpeed = CutSpeed + (2 * CutSens);
    if (CutSpeed > 125)
    {
      CutSens = -1;
    }
    if (CutSpeed < -125)
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