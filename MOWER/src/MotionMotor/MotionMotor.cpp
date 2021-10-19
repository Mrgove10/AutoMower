#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MotionMotor/MotionMotor.h"
#include "IOExtender/IOExtender.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Motion Motor Setup function
 */
void MotionMotorSetup(void)
{
  IOExtend.pinMode(PIN_MCP_MOTOR_RIGHT_LN1, OUTPUT);
  IOExtend.pinMode(PIN_MCP_MOTOR_RIGHT_LN2, OUTPUT);
  IOExtend.pinMode(PIN_MCP_MOTOR_LEFT_LN1, OUTPUT);
  IOExtend.pinMode(PIN_MCP_MOTOR_LEFT_LN2, OUTPUT);

  //  adcAttachPin(PIN_ESP_SPARE_1);                    // TEMPORARY FOR TESTS

  // configure LED PWM functionalitites
  ledcSetup(MOTION_MOTOR_PWM_CHANNEL_RIGHT, MOTION_MOTOR_PWM_FREQUENCY, MOTION_MOTOR_PWM_RESOLUTION);
  ledcSetup(MOTION_MOTOR_PWM_CHANNEL_LEFT, MOTION_MOTOR_PWM_FREQUENCY, MOTION_MOTOR_PWM_RESOLUTION);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PIN_ESP_MOTOR_RIGHT_PWM, MOTION_MOTOR_PWM_CHANNEL_RIGHT);
  ledcAttachPin(PIN_ESP_MOTOR_LEFT_PWM, MOTION_MOTOR_PWM_CHANNEL_LEFT);

  MotionMotorStop(MOTION_MOTOR_RIGHT);
  MotionMotorStop(MOTION_MOTOR_LEFT);

  DebugPrintln("Motion Motor setup Done", DBG_VERBOSE, true);
}

/**
 * Motion Motor Start function
 * @param Motor to start
 * @param Direction to set speed
 * @param Speed to set
 */
void MotionMotorStart(const int Motor, const int Direction, const int Speed)
{
  // check to see if motor is not already running in a different direction. If it is the case, stop the motor.

  if (g_MotionMotorOn[Motor] && g_MotionMotorDirection[Motor] != Direction)
  {
    MotionMotorStop(Motor);
  }

  if (Direction == MOTION_MOTOR_FORWARD)
  {
    IOExtendProtectedWrite(g_MotionMotorIn1Pin[Motor], LOW);
    IOExtendProtectedWrite(g_MotionMotorIn2Pin[Motor], HIGH);

    MotionMotorSetSpeed(Motor, Speed);
    g_MotionMotorOn[Motor] = true;
    g_MotionMotorDirection[Motor] = MOTION_MOTOR_FORWARD;
    DebugPrintln("Motion Motor " + g_MotionMotorStr[Motor] + " start Forward", DBG_VERBOSE, true);
  }

  if (Direction == MOTION_MOTOR_REVERSE)
  {
    IOExtendProtectedWrite(g_MotionMotorIn1Pin[Motor], HIGH);
    IOExtendProtectedWrite(g_MotionMotorIn2Pin[Motor], LOW);

    MotionMotorSetSpeed(Motor, Speed);
    g_MotionMotorOn[Motor] = true;
    g_MotionMotorDirection[Motor] = MOTION_MOTOR_REVERSE;
    DebugPrintln("Motion Motor " + g_MotionMotorStr[Motor] + " start Reverse", DBG_VERBOSE, true);
  }
}

/**
 * Motion Motor speed setting function. Speed can be set either in absolute or relative values depending on call parameters. Relative speed can either be positive or negative
 * @param Motor to set speed (in %)
 * @param Speed to set or to change
 * @param Relative boolean indicating if Speed parameter is given in relative or absolute value
 */
void MotionMotorSetSpeed(const int Motor, const int Speed, const bool Relative)
{
  static int previousSpeed[MOTION_MOTOR_COUNT] = {0, 0};
  int adjustedSpeed;

  // Establish new requested speed
  if (Relative)
  {
    adjustedSpeed = g_MotionMotorSpeed[Motor] + Speed + int(g_WheelPerimeterTrackingCorrection[Motor]);
  }
  else
  {
    adjustedSpeed = Speed + int(g_WheelPerimeterTrackingCorrection[Motor]);
  }

  // Check new requested speed for range and convert into PWM points
  int checkedspeed = max(0, adjustedSpeed);                                 // make sure speed is in 0-100% range
  checkedspeed = min(100, checkedspeed);                                    // make sure speed is in 0-100% range

  // if in relative mode, if calculated speed is less than motor min speed, then we use the minimum speed
  if (Relative && checkedspeed < MOTION_MOTOR_MIN_SPEED)
  {
    DebugPrintln("\t\t\t\t\tMotion Motor " + g_MotionMotorStr[Motor] + " speed " + String(checkedspeed) + " limited to minimum: " + String(MOTION_MOTOR_MIN_SPEED), DBG_DEBUG, true);
    checkedspeed = MOTION_MOTOR_MIN_SPEED;
  }

  int SpeedPoints = int(map(checkedspeed, 0, 100, 0, MOTION_MOTOR_POINTS)); // convert speed (in %) into PWM range

  // If new requested speed is different from current speed, apply the change if above minimum threshold
  if ((checkedspeed < MOTION_MOTOR_MIN_SPEED) && (checkedspeed != 0))
  {
    if (SpeedPoints != previousSpeed[Motor])
    {
      previousSpeed[Motor] = SpeedPoints;
      DebugPrintln("\t\t\t\t\tMotion Motor " + g_MotionMotorStr[Motor] + " speed " + String(checkedspeed) + " too low : 0 applied", DBG_DEBUG, true);
    }
    ledcWrite(g_MotionMotorPWMChannel[Motor], 0);
    g_MotionMotorSpeed[Motor] = 0;
  }
  else
  {
    ledcWrite(g_MotionMotorPWMChannel[Motor], SpeedPoints);
    g_MotionMotorSpeed[Motor] = checkedspeed;
    if (SpeedPoints != previousSpeed[Motor])
    {
      previousSpeed[Motor] = SpeedPoints;
      DebugPrintln("\t\t\t\t\tMotion Motor " + g_MotionMotorStr[Motor] + "\t@ " + String(checkedspeed) + "% (" + String(SpeedPoints) + ")", DBG_DEBUG, true);
    }
  }
}

/**
 * Motion Motor Stop function
 * @param Motor to stop
 */
void MotionMotorStop(const int Motor)
{
  IOExtendProtectedWrite(g_MotionMotorIn1Pin[Motor], LOW);
  IOExtendProtectedWrite(g_MotionMotorIn2Pin[Motor], LOW);
  g_WheelPerimeterTrackingCorrection[Motor] = 0;
  MotionMotorSetSpeed(Motor, 0);
  g_MotionMotorOn[Motor] = false;
  g_MotionMotorDirection[Motor] = MOTION_MOTOR_STOPPED;
  DebugPrintln("Motion Motor " + g_MotionMotorStr[Motor] + " Stopped", DBG_VERBOSE, true);
}

/**
 * Motion Motor test function
 * @param Motor to test
 */
void MotionMotorTest(const int Motor)
{
  DebugPrintln("Motion Motor Test for " + g_MotionMotorStr[Motor] + " started", DBG_INFO, true);

  if (Motor == 0)
  {
    DisplayClear();
    DisplayPrint(0, 0, F("Motion Motor Test"));
  }
  DisplayPrint(2, 2 + Motor, g_MotionMotorStr[Motor]);

  //Forward

  DisplayPrint(8, 2 + Motor, "Crawl FWD ");
  MotionMotorStart(Motor, MOTION_MOTOR_FORWARD, MOWER_MOVES_SPEED_CRAWL);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Slow FWD  ", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_SLOW);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Normal FWD", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_NORMAL);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Max FWD  ", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_MAX);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Slow FWD  ", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_SLOW);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  //Reverse

  MotionMotorStop(Motor);
  delay(250);

  DisplayPrint(8, 2 + Motor, "Crawl REV ", true);
  MotionMotorStart(Motor, MOTION_MOTOR_REVERSE, MOWER_MOVES_SPEED_CRAWL);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Slow REV  ", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_SLOW);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Normal REV", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_NORMAL);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Max REV  ", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_MAX);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Slow REV  ", true);
  MotionMotorSetSpeed(Motor, MOWER_MOVES_SPEED_SLOW);
  SerialAndTelnet.handle();
  delay(MOTION_MOTOR_TEST_STEP_DURATION);

  DisplayPrint(8, 2 + Motor, "Stopped   ", true);
  MotionMotorStop(Motor);
  SerialAndTelnet.handle();
  delay(TEST_SEQ_STEP_WAIT);

  if (Motor != 0)
  {
    DisplayClear();
  }
}