#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "CutMotor/CutMotor.h"
#include "IOExtender/IOExtender.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Cut Motor Setup function
 */
void CutMotorSetup()
{
  IOExtend.pinMode(PIN_MCP_MOTOR_CUT_LN1, OUTPUT);
  IOExtend.pinMode(PIN_MCP_MOTOR_CUT_LN2, OUTPUT);
  IOExtend.pinMode(PIN_MCP_MOTOR_CUT_HIGH_AMP, INPUT_PULLUP);

  // configure LED PWM functionalitites
  ledcSetup(CUT_MOTOR_PWM_CHANNEL_FORWARD, CUT_MOTOR_PWM_FREQUENCY, CUT_MOTOR_PWM_RESOLUTION);
  ledcSetup(CUT_MOTOR_PWM_CHANNEL_REVERSE, CUT_MOTOR_PWM_FREQUENCY, CUT_MOTOR_PWM_RESOLUTION);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PIN_ESP_MOTOR_CUT_PWM_FORWARD, CUT_MOTOR_PWM_CHANNEL_FORWARD);
  ledcAttachPin(PIN_ESP_MOTOR_CUT_PWM_REVERSE, CUT_MOTOR_PWM_CHANNEL_REVERSE);

  CutMotorStop();

  DebugPrintln("Cut Motor setup Done", DBG_VERBOSE, true);
}

/**
 * Cut Motor Start function
 * @param Direction to set speed
 * @param Speed to set
 */
void CutMotorStart(const int Direction, const int Speed)
{
  // check to see if motor is not already running in a different direction. If it is the case, stop the motor.

  if (g_CutMotorOn && g_CutMotorDirection != Direction)
  {
    CutMotorStop(true);
  }

  if (Direction == CUT_MOTOR_FORWARD)
  {
    g_CutMotorOn = true;
    g_CutMotorDirection = CUT_MOTOR_FORWARD;
    CutMotorSetSpeed(Speed);
    DebugPrintln("Cut Motor start Forward", DBG_VERBOSE, true);
  }

  if (Direction == CUT_MOTOR_REVERSE)
  {
    g_CutMotorOn = true;
    g_CutMotorDirection = CUT_MOTOR_REVERSE;
    CutMotorSetSpeed(Speed);
    DebugPrintln("Cut Motor start Reverse", DBG_VERBOSE, true);
  }
  IOExtendProtectedWrite(PIN_MCP_MOTOR_CUT_LN1, HIGH);
  IOExtendProtectedWrite(PIN_MCP_MOTOR_CUT_LN2, HIGH);
}

/**
 * Cut Motor Stop function
 * @param Immediate optional bool to force a fast motor stop
 */
void CutMotorStop(const bool Immedate)
{
  int CutSpeed = g_CutMotorSpeed;

  CutMotorSetSpeed(0);

  if (Immedate)
  {
    // A fast stop is acheived by injecting a brief opposite direction rotation order

    if (g_CutMotorDirection == CUT_MOTOR_FORWARD)
    {
      ledcWrite(CUT_MOTOR_PWM_CHANNEL_REVERSE, min(CUT_MOTOR_FAST_STOP_INVERSE_SPEED, CutSpeed));
      delay(CUT_MOTOR_FAST_STOP_INVERSE_DURATION);
      ledcWrite(CUT_MOTOR_PWM_CHANNEL_REVERSE, 0);
    }
    if (g_CutMotorDirection == CUT_MOTOR_REVERSE)
    {
      ledcWrite(CUT_MOTOR_PWM_CHANNEL_FORWARD, min(CUT_MOTOR_FAST_STOP_INVERSE_SPEED, CutSpeed));
      delay(CUT_MOTOR_FAST_STOP_INVERSE_DURATION);
      ledcWrite(CUT_MOTOR_PWM_CHANNEL_FORWARD, 0);
    }
    DebugPrintln("Cut Motor IMMEDIATE Stop requested", DBG_DEBUG, true);
  }

  IOExtendProtectedWrite(PIN_MCP_MOTOR_CUT_LN1, LOW);
  IOExtendProtectedWrite(PIN_MCP_MOTOR_CUT_LN2, LOW);

  g_CutMotorOn = false;
  g_CutMotorDirection = CUT_MOTOR_STOPPED;
  DebugPrintln("Cut Motor Stopped", DBG_DEBUG, true);
}

/**
 * Cut Motor speed setting function
 * @param Speed to set
 */
void CutMotorSetSpeed(const int Speed)
{
  if (Speed > 0 && Speed <= 100)
  {
    if ((Speed < CUT_MOTOR_MIN_SPEED) && (Speed != 0))
    {
      DebugPrintln("Cut Motor speed " + String(Speed) + " too low : not applied", DBG_VERBOSE, true);

      if (g_CutMotorDirection == CUT_MOTOR_FORWARD)
      {
        ledcWrite(CUT_MOTOR_PWM_CHANNEL_FORWARD, 0);
      }
      if (g_CutMotorDirection == CUT_MOTOR_REVERSE)
      {
        ledcWrite(CUT_MOTOR_PWM_CHANNEL_REVERSE, 0);
      }
      g_CutMotorSpeed = 0;
    }
    else
    {
      DebugPrintln("Cut Motor @ " + String(Speed), DBG_VERBOSE, true);

      if (g_CutMotorDirection == CUT_MOTOR_FORWARD)
      {
        ledcWrite(CUT_MOTOR_PWM_CHANNEL_FORWARD, (uint32_t)(Speed * 4096) / 100);
      }
      if (g_CutMotorDirection == CUT_MOTOR_REVERSE)
      {
        ledcWrite(CUT_MOTOR_PWM_CHANNEL_REVERSE, (uint32_t) (Speed * 4096) /100);
      }
      g_CutMotorSpeed = Speed /100 * 4096;
    }
  }
};

/**
 * Cut Motor test function
 */
void CutMotorTest(void)
{
  DebugPrintln("Cut Motor Test for started", DBG_INFO, true);

  DisplayClear();
  DisplayPrint(0, 0, F("Cut Motor Test"));

#define CRAWL 10
#define SLOW 30
#define NORMAL 70
#define FAST 100
#define DURATION 2000

  //Forward

  DisplayPrint(4, 2, "Crawl FWD ");
  CutMotorStart(MOTION_MOTOR_FORWARD, CRAWL);
  SerialAndTelnet.handle();
  delay(DURATION);

  DisplayPrint(4, 2, "Slow FWD  ", true);
  CutMotorSetSpeed(SLOW);
  SerialAndTelnet.handle();
  delay(DURATION);

  DisplayPrint(4, 2, "Normal FWD", true);
  CutMotorSetSpeed(NORMAL);
  SerialAndTelnet.handle();
  delay(DURATION);

  DisplayPrint(4, 2, "Fast FWD  ", true);
  CutMotorSetSpeed(FAST);
  SerialAndTelnet.handle();
  delay(DURATION*5);

  DisplayPrint(4, 2, "Slow FWD  ", true);
  CutMotorSetSpeed(SLOW);
  SerialAndTelnet.handle();
  delay(2 * DURATION);

  //Reverse

  CutMotorStop(true);
  delay(DURATION);

  DisplayPrint(4, 2, "Crawl REV ", true);
  CutMotorStart(MOTION_MOTOR_REVERSE, CRAWL);
  SerialAndTelnet.handle();
  delay(DURATION);

  DisplayPrint(4, 2, "Slow REV  ", true);
  CutMotorSetSpeed(SLOW);
  SerialAndTelnet.handle();
  delay(DURATION);

  DisplayPrint(4, 2, "Normal REV", true);
  CutMotorSetSpeed(NORMAL);
  SerialAndTelnet.handle();
  delay(DURATION);

  DisplayPrint(4, 2, "Fast REV  ", true);
  CutMotorSetSpeed(FAST);
  SerialAndTelnet.handle();
  delay(DURATION*5);

  DisplayPrint(4, 2, "Slow REV  ", true);
  CutMotorSetSpeed(SLOW);
  SerialAndTelnet.handle();
  delay(DURATION);

  // Stop

  DisplayPrint(4, 2, "Stopped   ", true);
  CutMotorStop();
  SerialAndTelnet.handle();
  delay(TEST_SEQ_STEP_WAIT);

  DisplayClear();
}

/**
 * Cut Motor Check function
 * @param Now true to force immediate check
 */
void CutMotorCheck(const bool Now)
{
  static unsigned long LastCutMotorCheck = 0;

  if ((millis() - LastCutMotorCheck > CUT_MOTOR_CHECK_INTERVAL) || Now)
  {
    g_CutMotorAlarm = (IOExtendProtectedRead(PIN_MCP_MOTOR_CUT_HIGH_AMP) == 1);
    // DebugPrintln("Cut Motor Status: " + String(g_CutMotorAlarm), DBG_VERBOSE, true);
    LastCutMotorCheck = millis();
  }
}