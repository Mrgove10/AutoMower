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

  /*  DebugPrintln("loop Always", DBG_ALWAYS, true);
  DebugPrintln("loop Error", DBG_ERROR, true);
  DebugPrintln("loop Warning", DBG_WARNING, true);
  DebugPrintln("loop Info", DBG_INFO, true);
  DebugPrintln("loop Debug", DBG_DEBUG, true);
  DebugPrintln("loop Verbose", DBG_VERBOSE, true);

  TestVal1 = TestVal1 + 1;
  TestVal2 = TestVal2 + 2;
  TestVal3 = TestVal3 + 3;
  TestVal4 = TestVal4 + 4;
  
  DebugPrint("TestVal1=" + String(TestVal1), DBG_INFO, true);
  DebugPrint(" Val2=" + String(TestVal2));
  DebugPrint(" Val3=" + String(TestVal3));
  DebugPrintln(" Val4=" + String(TestVal4));
*/
  EEPROMSave(false);

  if (RightBumperTriggered)
  {
    DebugPrintln("Right Bumper Triggered !", DBG_INFO, true);
    RightBumperTriggered = false;
  }
  if (LeftBumperTriggered)
  {
    DebugPrintln("Left Bumper Triggered !", DBG_INFO, true);
    LeftBumperTriggered = false;
  }

  if (HorizontalTiltTriggered)
  {
    DebugPrintln("Horizontal Tilt sensor Triggered !", DBG_INFO, true);
    HorizontalTiltTriggered = false;
  }

  if (VerticalTiltTriggered)
  {
    DebugPrintln("Vertical Tilt sensor Triggered !", DBG_INFO, true);
    VerticalTiltTriggered = false;
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

  FanCheck(FAN_1_RED);
  FanCheck(FAN_2_BLUE);

  CutMotorCheck();

  static unsigned long LastRefresh = 0;

  if ((millis() - LastRefresh > 500))
  {

// Motion motor loop
    MotionSpeed = MotionSpeed + (12 * MotionSens);
    if (MotionSpeed > 4096 + 1024)
    {
      MotionSens = -1;
    }
    if (MotionSpeed < -4096 - 1024)
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
    if (!MotionMotorOn[MOTION_MOTOR_RIGHT])
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
    CutSpeed = CutSpeed + (16 * CutSens);
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
    if (!CutMotorOn)
    {
      CutMotorStart(CutDirection, abs(CutSpeed));
    }
    else
    {
      CutMotorSetSpeed(abs(CutSpeed));
    }
  }

  SerialAndTelnet.handle();

  if ((millis() - LastRefresh > 500))
  {
    DebugPrint("Temp 1: " + String(Temperature[TEMPERATURE_1_RED], 1) +         // " | Err1: " + String(Temp1ErrorCount) +
                   " |Temp 2: " + String(Temperature[TEMPERATURE_2_BLUE], 1) + //" | Err2: " + String(Temp2ErrorCount) +
                   " |Charge: " + String(BatteryChargeCurrent, 0) +
                   " |MotorR: " + String(MotorCurrent[MOTOR_CURRENT_RIGHT], 2) +
                   " |MotorL: " + String(MotorCurrent[MOTOR_CURRENT_LEFT], 2) +
                   " |MotorC: " + String(MotorCurrent[MOTOR_CURRENT_CUT], 2) +
                   " |MotorCAlm: " + String(CutMotorAlarm) +
                   " |Volt: " + String(float(BatteryVotlage) / 1000.0f, 2) +
                   " |Heading: " + String(CompassHeading, 1),
               DBG_INFO, true);

//    DisplayClear();
    DisplayPrint(0, 0, "T1: " + String(Temperature[TEMPERATURE_1_RED], 1) + " T2: " + String(Temperature[TEMPERATURE_2_BLUE], 1), true);

    for (uint8_t i = 0; i < SONAR_COUNT; i++)
    {            // Loop through each sensor and display results.
      DebugPrint(" | Sonar" + String(i + 1) + ": " + String(SonarDistance[i]));
      DisplayPrint(0 + i * 6, 1, "S" + String(i + 1) + ":" + String(SonarDistance[i]) + " ", true);
    }
    DebugPrintln("");
    LastRefresh = millis();
  }
  for (int i = 0; i < KEYPAD_MAX_KEYS; i++)
  {
    //    if (!key) {DebugPrintln("Keypad key" + String(i-7) + " pressed", DBG_INFO, true);}
    DisplayPrint(0 + i * 5, 2, "K" + String(i + 1) + ":" + String(KeyPressed[i]) + " ", true);
  }

  /*
  for (uint8_t i = 8; i < 12; i++){
    int key = IOExtend.digitalRead(i);
    if (!key) {DebugPrintln("Keypad key" + String(i-7) + " pressed", DBG_INFO, true);}
    lcd.print("K" + String(i-7) + ":" + String(key) + " ");
  }
*/

//  DisplayPrint(0, 3, "B:" + String(BatteryChargeCurrent, 0) + " ", true);
  DisplayPrint(0, 3, "R:" + String(MotorCurrent[MOTOR_CURRENT_RIGHT], 0) + " ", true);
  DisplayPrint(6, 3 , "L:" + String(MotorCurrent[MOTOR_CURRENT_LEFT], 0) + " ", true);
  DisplayPrint(12, 3 , "C:" + String(MotorCurrent[MOTOR_CURRENT_CUT], 0) + " ", true);
  
  MQTTReconnect();

  MQTTSendTelemetry();

  MQTTclient.loop();

  SerialAndTelnet.handle();

  events(); // eztime refresh

  delay(50);
}