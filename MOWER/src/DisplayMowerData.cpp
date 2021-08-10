#include <Arduino.h>
#include "myGlobals_definition.h"
#include "mySetup.h"
#include "Utils/Utils.h"
#include "MQTT/MQTT.h"
#include "Temperature/Temperature.h"
#include "Current/Current.h"
#include "Voltage/Voltage.h"
#include "Sonar/Sonar.h"
#include "Bumper/Bumper.h"
#include "Tilt/Tilt.h"
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
  if (BumperRead(BUMPER_RIGHT))
  {
    DebugPrintln("Right Bumper Triggered !", DBG_INFO, true);
  }
  if (BumperRead(BUMPER_LEFT))
  {
    DebugPrintln("Left Bumper Triggered !", DBG_INFO, true);
  }

  if (TiltRead(TILT_HORIZONTAL))
  {
    DebugPrintln("Horizontal Tilt sensor Triggered !", DBG_INFO, true);
  }

  if (TiltRead(TILT_VERTICAL))
  {
    DebugPrintln("Vertical Tilt sensor Triggered !", DBG_INFO, true);
  }

  BatteryChargeCurrentRead(false);
  MotorCurrentRead(MOTOR_CURRENT_RIGHT);
  MotorCurrentRead(MOTOR_CURRENT_LEFT);
  MotorCurrentRead(MOTOR_CURRENT_CUT);

  KeypadRead();

  SonarRead(SONAR_FRONT, true);   // force sonar read as reading task may not be activated
  SonarRead(SONAR_LEFT); // force sonar read as reading task may not be activated
  SonarRead(SONAR_RIGHT); // force sonar read as reading task may not be activated

  BatteryVoltageRead();

  CompassRead();

  GPSRead(true);

  CutMotorCheck();

  static unsigned long LastRefreshed = 0;

  if ((millis() - LastRefreshed > MOWER_DATA_DISPLAY_INTERVAL))
  {
    DebugPrint("T1:" + String(g_Temperature[TEMPERATURE_1_RED], 1) +        // " | Err1: " + String(Temp1ErrorCount) +
                   " |T2:" + String(g_Temperature[TEMPERATURE_2_BLUE], 1) + //" | Err2: " + String(Temp2ErrorCount) +
                   " |Charge:" + String(g_BatteryChargeCurrent, 0) +
                   " |MR:" + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 1) +
                   " |ML:" + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 1) +
                   " |MC:" + String(g_MotorCurrent[MOTOR_CURRENT_CUT], 1) +
                   " |MCAlm:" + String(g_CutMotorAlarm) +
                   " |Volt:" + String(float(g_BatteryVotlage) / 1000.0f, 1) +
                   " |Head:" + String(g_CompassHeading, 1) +
                   " |Timouts:" + String(g_FastAnaReadTimeout) +
                   " |MaxQ:" + String(g_inQueueMax),
               DBG_VERBOSE, true);

    g_FastAnaReadTimeout = 0;
    g_inQueueMax = 0;
    g_inQueue = 0;

    //    DisplayClear();
    DisplayPrint(0, 0, "T1: " + String(g_Temperature[TEMPERATURE_1_RED], 1) + " T2: " + String(g_Temperature[TEMPERATURE_2_BLUE], 1), true);

    for (uint8_t i = 0; i < SONAR_COUNT; i++)
    {            // Loop through each sensor and display results.
      DebugPrint(" | " + g_sensorStr[i] + ":" + String(g_SonarDistance[i]),DBG_VERBOSE, false, true);
      DisplayPrint(0 + i * 6, 1, "S" + String(i + 1) + ":" + String(g_SonarDistance[i]) + " ", true);
    }
    DebugPrintln("",DBG_VERBOSE,false,true);
    LastRefreshed = millis();
  }
  for (int i = 0; i < KEYPAD_MAX_KEYS; i++)
  {
    //    if (!key) {DebugPrintln("Keypad key" + String(i-7) + " pressed", DBG_INFO, true);}
    DisplayPrint(0 + i * 5, 2, "K" + String(i + 1) + ":" + String(g_KeyPressed[i]) + " ", true);
  }

  //  DisplayPrint(0, 3, "B:" + String(g_BatteryChargeCurrent, 0) + " ", true);
  DisplayPrint(0, 3, "R:" + String(g_MotorCurrent[MOTOR_CURRENT_RIGHT], 0) + " ", true);
  DisplayPrint(6, 3, "L:" + String(g_MotorCurrent[MOTOR_CURRENT_LEFT], 0) + " ", true);
  DisplayPrint(12, 3, "C:" + String(g_MotorCurrent[MOTOR_CURRENT_CUT], 0) + " ", true);
}