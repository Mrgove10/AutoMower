#include <Arduino.h>
#include "myGlobals_definition.h"
#include "mySetup.h"
#include "Utils/Utils.h"
#include "MQTT/MQTT.h"
#include "Temperature/Temperature.h"
#include "Current/Current.h"
#include "Voltage/Voltage.h"
#include "Sonar/Sonar.h"
#include "Keypad/Keypad.h"
#include "Compass/Compass.h"
#include "GPS/GPS.h"
#include "IOExtender/IOExtender.h"
#include "MotionMotor/MotionMotor.h"
#include "CutMotor/CutMotor.h"
#include <pin_definitions.h>
#include "Display/Display.h"

void DisplayMowerData()
{
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

  SonarRead(SONAR_FRONT);
  SonarRead(SONAR_LEFT);
  SonarRead(SONAR_RIGHT);

  BatteryVoltageRead();

  CompassRead();

  GPSRead(true);

  CutMotorCheck();

  static unsigned long LastRefreshed = 0;

  if ((millis() - LastRefreshed > MOWER_DATA_DISPLAY_INTERVAL))
  {
    DebugPrint("Temp 1: " + String(g_Temperature[TEMPERATURE_1_RED], 1) +         // " | Err1: " + String(Temp1ErrorCount) +
                   " |Temp 2: " + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + //" | Err2: " + String(Temp2ErrorCount) +
                   " |Charge: " + String(g_BatteryChargeCurrent, 0) +
                   " |MotorR: " + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 1) +
                   " |MotorL: " + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 1) +
                   " |MotorC: " + String(g_MotorCurrent[MOTOR_CURRENT_CUT], 1) +
                   " |MotorCAlm: " + String(g_CutMotorAlarm) +
                   " |Volt: " + String(float(g_BatteryVotlage) / 1000.0f, 1) +
                   " |Heading: " + String(g_CompassHeading, 1),
               DBG_INFO, true);

//    DisplayClear();
    DisplayPrint(0, 0, "T1: " + String(g_Temperature[TEMPERATURE_1_RED], 1) + " T2: " + String(g_Temperature[TEMPERATURE_2_BLUE], 1), true);

    for (uint8_t i = 0; i < SONAR_COUNT; i++)
    {            // Loop through each sensor and display results.
      DebugPrint(" | Sonar" + String(i + 1) + ": " + String(g_SonarDistance[i]));
      DisplayPrint(0 + i * 6, 1, "S" + String(i + 1) + ":" + String(g_SonarDistance[i]) + " ", true);
    }
    DebugPrintln("");
    LastRefreshed = millis();
  }
  for (int i = 0; i < KEYPAD_MAX_KEYS; i++)
  {
    //    if (!key) {DebugPrintln("Keypad key" + String(i-7) + " pressed", DBG_INFO, true);}
    DisplayPrint(0 + i * 5, 2, "K" + String(i + 1) + ":" + String(g_KeyPressed[i]) + " ", true);
  }

//  DisplayPrint(0, 3, "B:" + String(g_BatteryChargeCurrent, 0) + " ", true);
  DisplayPrint(0, 3, "R:" + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 0) + " ", true);
  DisplayPrint(6, 3 , "L:" + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 0) + " ", true);
  DisplayPrint(12, 3 , "C:" + String(g_MotorCurrent[MOTOR_CURRENT_CUT], 0) + " ", true);
}