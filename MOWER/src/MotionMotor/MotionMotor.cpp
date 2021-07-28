#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MotionMotor/MotionMotor.h"
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
    IOExtend.digitalWrite(g_MotionMotorIn1Pin[Motor], LOW);
    IOExtend.digitalWrite(g_MotionMotorIn2Pin[Motor], HIGH);
    MotionMotorSetSpeed(Motor, Speed);
    g_MotionMotorOn[Motor] = true;
    g_MotionMotorDirection[Motor] = MOTION_MOTOR_FORWARD;
    DebugPrintln("Motion Motor " + g_MotionMotorStr[Motor] + " start Forward", DBG_VERBOSE, true);
  }

  if (Direction == MOTION_MOTOR_REVERSE)
  {
    IOExtend.digitalWrite(g_MotionMotorIn1Pin[Motor], HIGH);
    IOExtend.digitalWrite(g_MotionMotorIn2Pin[Motor], LOW);
    MotionMotorSetSpeed(Motor, Speed);
    g_MotionMotorOn[Motor] = true;
    g_MotionMotorDirection[Motor] = MOTION_MOTOR_REVERSE;
    DebugPrintln("Motion Motor " + g_MotionMotorStr[Motor] + " start Reverse", DBG_VERBOSE, true);
  }
}

/**
 * Motion Motor speed setting function
 * @param Motor to set speed (in %)
 * @param Speed to set
 */
void MotionMotorSetSpeed(const int Motor, const int Speed)
{
  int checkedspeed = max(0,Speed);          // make sure speed is in 0-100% range
  checkedspeed = min(100, Speed);          // make sure speed is in 0-100% range
  int SpeedPoints = int(map(checkedspeed,0,100,0,MOTION_MOTOR_POINTS));      // convert speed (in %) into PWM range

  if ((Speed < MOTION_MOTOR_MIN_SPEED) && (Speed != 0))
  {
    DebugPrintln("Motion Motor " + g_MotionMotorStr[Motor] + " speed " + String(checkedspeed) + " too low : not applied", DBG_VERBOSE, true);
    ledcWrite(g_MotionMotorPWMChannel[Motor], 0);
    g_MotionMotorSpeed[Motor] = 0;
  }
  else
  {
    ledcWrite(g_MotionMotorPWMChannel[Motor], SpeedPoints);
    g_MotionMotorSpeed[Motor] = checkedspeed;

    DebugPrintln("Motion Motor " + g_MotionMotorStr[Motor] + " @ " + String(checkedspeed) + "% on Channel " + String(g_MotionMotorPWMChannel[Motor]) + " (" + String(SpeedPoints) + ")", DBG_VERBOSE, true);
  }
};

/**
 * Motion Motor Stop function
 * @param Motor to stop
 */
void MotionMotorStop(const int Motor)
{
  IOExtend.digitalWrite(g_MotionMotorIn1Pin[Motor], LOW);
  IOExtend.digitalWrite(g_MotionMotorIn2Pin[Motor], LOW);
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

#define MOTION_MOTOR_TEST_STEP_DURATION 2000

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