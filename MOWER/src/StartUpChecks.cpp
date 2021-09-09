#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "StartupChecks.h"
#include "Utils/Utils.h"
#include "Rain/Rain.h"
#include "Bumper/Bumper.h"
#include "Battery/Battery.h"
#include "Tilt/Tilt.h"
#include "Temperature/Temperature.h"
#include "Sonar/Sonar.h"
#include "MotorCurrent/MotorCurrent.h"
#include "Fan/Fan.h"
#include "Compass/Compass.h"
#include "GPS/GPS.h"
#include "MotionMotor/MotionMotor.h"
#include "CutMotor/CutMotor.h"
#include "GyroAccel/GyroAccel.h"
#include "Buzzer/Buzzer.h"
#include "Display/Display.h"

/**
 * Runs all Mower checks on Startup
 * @param true to indicate that all tests are to be performed of false to perform minimum tests (false by default)
 * @return true if all checks pass
 */
bool StartupChecks(const bool allTests)
{
  bool allChecks = true;

  DebugPrintln("");
  DebugPrintln("Starting AutoMower startup checks.....", DBG_INFO, true);
  DebugPrintln("");
  DisplayClear();
  DisplayPrint(0, 1, F("Startup Checks..."));
  delay(TEST_SEQ_STEP_WAIT);

  if (!BatteryVoltageCheck())
  {
    allChecks = false;
  };
  DebugPrintln(" ");

  if (!TiltSensorCheck(TILT_HORIZONTAL))
  {
    allChecks = false;
  };
  if (!TiltSensorCheck(TILT_VERTICAL))
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!BumperSensorCheck(BUMPER_LEFT))
  {
    allChecks = false;
  };
  if (!BumperSensorCheck(BUMPER_RIGHT))
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!RainSensorCheck())
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!TemperatureSensorCheck(TEMPERATURE_1_RED))
  {
    allChecks = false;
  };
  if (!TemperatureSensorCheck(TEMPERATURE_2_BLUE))
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!SonarSensorCheck(SONAR_FRONT))
  {
    allChecks = false;
  };
  if (!SonarSensorCheck(SONAR_LEFT))
  {
    allChecks = false;
  };
  if (!SonarSensorCheck(SONAR_RIGHT))
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!BatteryCurrentSensorCheck())
  {
    allChecks = false;
  };
  if (!MotorCurrentSensorCheck(MOTOR_CURRENT_RIGHT))
  {
    allChecks = false;
  };
  if (!MotorCurrentSensorCheck(MOTOR_CURRENT_LEFT))
  {
    allChecks = false;
  };
  if (!MotorCurrentSensorCheck(MOTOR_CURRENT_CUT))
  {
    allChecks = false;
  };

  if (allTests)
  {
    DebugPrintln(" ");
    FanTest(FAN_1_RED);
    FanTest(FAN_2_BLUE);
  }
  DebugPrintln(" ");

  if (!CompassSensorCheck())
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!GyroAccelCheck())
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  if (!GPSCheck())
  {
    allChecks = false;
  };

  DebugPrintln(" ");

  //  MotionMotorTest(MOTION_MOTOR_RIGHT); //TEMPORAIRE
  //  DebugPrintln(" ");
  //  MotionMotorTest(MOTION_MOTOR_LEFT);  //TEMPORAIRE
  //  DebugPrintln(" ");

  //  CutMotorTest(); //TEMPORAIRE
  //  DebugPrintln(" ");

  // insert here all other startup checks

  if (allChecks)
  {
    LogPrintln("All checks Ok", TAG_CHECK, DBG_ALWAYS);
  }
  DebugPrintln("");

  DebugPrintln("");
  DebugPrintln("End of AutoMower startup checks.....", DBG_INFO, true);
  DebugPrintln("");

  playTune(g_readyTune, sizeof(g_readyTune) / sizeof(noteStruct));

  SerialAndTelnet.handle();

  return allChecks;
}