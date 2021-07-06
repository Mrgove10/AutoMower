#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "MotionMotor/MotionMotor.h"
#include "Utils/Utils.h"

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

  if (MotionMotorOn[Motor] && MotionMotorDirection[Motor] != Direction) 
  {
    MotionMotorStop(Motor);
  }

  if (Direction == MOTION_MOTOR_FORWARD)
  {
      IOExtend.digitalWrite(MotionMotorIn1Pin[Motor],LOW);
      IOExtend.digitalWrite(MotionMotorIn2Pin[Motor],HIGH);
      MotionMotorSetSpeed(Motor, Speed);
      MotionMotorOn[Motor] = true;
      MotionMotorDirection[Motor] = MOTION_MOTOR_FORWARD;
      DebugPrintln("Motion Motor " + MotionMotorStr[Motor] + " start Forward", DBG_VERBOSE, true);
  }

  if (Direction == MOTION_MOTOR_REVERSE)
  {
      IOExtend.digitalWrite(MotionMotorIn1Pin[Motor],HIGH);
      IOExtend.digitalWrite(MotionMotorIn2Pin[Motor],LOW);
      MotionMotorSetSpeed(Motor, Speed);
      MotionMotorOn[Motor] = true;
      MotionMotorDirection[Motor] = MOTION_MOTOR_REVERSE;
      DebugPrintln("Motion Motor " + MotionMotorStr[Motor] + " start Reverse", DBG_VERBOSE, true);
  }
}

/**
 * Motion Motor speed setting function
 * @param Motor to set speed
 * @param Speed to set
 */
void MotionMotorSetSpeed(const int Motor, const int Speed)
{
  if (Speed > 0 && Speed < 4096) {
    if ((Speed < MOTION_MOTOR_MIN_SPEED) && (Speed != 0)) 
    {
      DebugPrintln("Motion Motor " + MotionMotorStr[Motor] + " speed " + String(Speed) + " too low : not applied", DBG_VERBOSE, true);
      ledcWrite(MotionMotorPWMChannel[Motor], 0);
      MotionMotorSpeed[Motor] = 0;

    }
    else 
    {
      ledcWrite(MotionMotorPWMChannel[Motor], Speed);
      MotionMotorSpeed[Motor] = Speed;

      DebugPrintln("Motion Motor " + MotionMotorStr[Motor] + " @ " + String(Speed) + " on Channel " + String(MotionMotorPWMChannel[Motor]), DBG_VERBOSE, true);
    }
  }  
};

/**
 * Motion Motor Stop function
 * @param Motor to stop
 */
void MotionMotorStop(const int Motor)
{
  IOExtend.digitalWrite(MotionMotorIn1Pin[Motor],LOW);
  IOExtend.digitalWrite(MotionMotorIn2Pin[Motor],LOW);
  MotionMotorSetSpeed(Motor, 0);
  MotionMotorOn[Motor] = false;
  MotionMotorDirection[Motor] = MOTION_MOTOR_STOPPED;
  DebugPrintln("Motion Motor " + MotionMotorStr[Motor] + " Stopped", DBG_VERBOSE, true);
}

/**
 * Motion Motor test function
 * @param Motor to test
 */
void MotionMotorTest(const int Motor)
{
  DebugPrintln("Motion Motor Test for " + MotionMotorStr[Motor] + " started", DBG_INFO, true);

  if (Motor == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Motion Motor Test"));
  }
  lcd.setCursor(2, 2 + Motor);
  lcd.print(MotionMotorStr[Motor]);
  lcd.setCursor(8, 2 + Motor);

  #define CRAWL 4096*25/100
  #define SLOW 4096*40/100
  #define NORMAL 4096*70/100
  #define FAST 4090
  #define DURATION 2000

//Forward

  lcd.print("CRAWL FWD ");
  MotionMotorStart(Motor, MOTION_MOTOR_FORWARD, CRAWL);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("SLOW FWD  ");
  MotionMotorSetSpeed(Motor, SLOW);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("NORMAL FWD");
  MotionMotorSetSpeed(Motor, NORMAL);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("FAST FWD  ");
  MotionMotorSetSpeed(Motor, FAST);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("SLOW FWD  ");
  MotionMotorSetSpeed(Motor, SLOW);
  SerialAndTelnet.handle();
  delay(DURATION);

//Reverse

  MotionMotorStop(Motor);
  delay(250);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("CRAWL REV ");
  MotionMotorStart(Motor, MOTION_MOTOR_REVERSE, CRAWL);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("SLOW REV  ");
  MotionMotorSetSpeed(Motor, SLOW);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("NORMAL REV");
  MotionMotorSetSpeed(Motor, NORMAL);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("FAST REV  ");
  MotionMotorSetSpeed(Motor, FAST);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("SLOW REV  ");
  MotionMotorSetSpeed(Motor, SLOW);
  SerialAndTelnet.handle();
  delay(DURATION);

  lcd.setCursor(8, 2 + Motor);
  lcd.print("STOPPED   ");
  MotionMotorStop(Motor);
  SerialAndTelnet.handle();
  delay(TEST_SEQ_STEP_WAIT);

  if (Motor != 0) {
    lcd.clear();
  }
}