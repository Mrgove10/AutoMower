
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
#include <pin_definitions.h>

void setup()
{
  MySetup();
}

void loop()
{

  static int speed = 0;
  static int sens = 1;
  static int direction = MOTION_MOTOR_STOPPED;

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
  //  MotorCurrentRead(MOTOR_CURRENT_LEFT);
  //  MotorCurrentRead(MOTOR_CURRENT_CUT);
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

  static unsigned long LastRefresh = 0;

  if ((millis() - LastRefresh > 500))
  {
    speed = speed + (8 * sens);
    if (speed > 4096 + 1024)
    {
      sens = -1;
    }
    if (speed < -4096 - 1024)
    {
      sens = 1;
    }
    if (speed < 0)
    {
      if (direction != MOTION_MOTOR_REVERSE)
      {
        direction = MOTION_MOTOR_REVERSE;
        MotionMotorStop(MOTION_MOTOR_RIGHT);
      }
    }
    else
    {
      if (direction != MOTION_MOTOR_FORWARD)
      {
        direction = MOTION_MOTOR_FORWARD;
        MotionMotorStop(MOTION_MOTOR_RIGHT);
      }
    }
    if (!MotionMotorOn[MOTION_MOTOR_RIGHT])
    {
      MotionMotorStart(MOTION_MOTOR_RIGHT, direction, abs(speed));
    }
    else
    {
      MotionMotorSetSpeed(MOTION_MOTOR_RIGHT, abs(speed));
    }
  }

  SerialAndTelnet.handle();

  if ((millis() - LastRefresh > 500))
  {
    DebugPrint("Temp 1: " + String(Temperature[TEMPERATURE_1_RED], 1) +         // " | Err1: " + String(Temp1ErrorCount) +
                   " | Temp 2: " + String(Temperature[TEMPERATURE_2_BLUE], 1) + //" | Err2: " + String(Temp2ErrorCount) +
                   " | Charge: " + String(BatteryChargeCurrent, 0) +
                   " | MotorR: " + String(MotorCurrent[MOTOR_CURRENT_RIGHT], 2) +
                   " | Volt: " + String(float(BatteryVotlage) / 1000.0f, 2) +
                   " | Heading: " + String(CompassHeading, 1),
               DBG_INFO, true);

    //  lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T1: " + String(Temperature[TEMPERATURE_1_RED], 1) + " T2: " + String(Temperature[TEMPERATURE_2_BLUE], 1));

    lcd.setCursor(0, 1);
    for (uint8_t i = 0; i < SONAR_COUNT; i++)
    {            // Loop through each sensor and display results.
      delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
      DebugPrint(" | Sonar" + String(i + 1) + ": " + String(SonarDistance[i]));
      lcd.print("S" + String(i + 1) + ":" + String(SonarDistance[i]) + " ");
    }
    DebugPrintln("");
    LastRefresh = millis();
  }
  lcd.setCursor(0, 2);
  for (int i = 0; i < KEYPAD_MAX_KEYS; i++)
  {
    //    if (!key) {DebugPrintln("Keypad key" + String(i-7) + " pressed", DBG_INFO, true);}
    lcd.print("K" + String(i + 1) + ":" + String(KeyPressed[i]) + " ");
  }

  /*
  for (uint8_t i = 8; i < 12; i++){
    int key = IOExtend.digitalRead(i);
    if (!key) {DebugPrintln("Keypad key" + String(i-7) + " pressed", DBG_INFO, true);}
    lcd.print("K" + String(i-7) + ":" + String(key) + " ");
  }
*/

  lcd.setCursor(0, 3);
  lcd.print("B:" + String(BatteryChargeCurrent, 0) + " ");
  lcd.print("R:" + String(MotorCurrent[MOTOR_CURRENT_RIGHT], 0) + " ");
  lcd.print("L:" + String(MotorCurrent[MOTOR_CURRENT_LEFT], 0) + " ");
  //  lcd.print("C:" + String(MotorCurrent[MOTOR_CURRENT_CUT],0) + " ");

  MQTTReconnect();

  MQTTSendTelemetry();

  MQTTclient.loop();

  SerialAndTelnet.handle();

  events(); // eztime refresh

  delay(50);
}