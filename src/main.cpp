
#include <Arduino.h>
#include "myGlobals_definition.h"
#include "mySetup.h"
#include "Utils/Utils.h"
#include "Temperature/Temperature.h"
#include "Current/Current.h"
#include "Sonar/Sonar.h"
#include "EEPROM/EEPROM.h"
#include "IOExtender/IOExtender.h"
#include <pin_definitions.h>

void setup()
{
  MySetup();
}

void loop()
{

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

  if (RightBumperTriggered) {
    DebugPrintln("Right Bumper Triggered !", DBG_INFO, true);
    RightBumperTriggered = false;
  }
  if (LeftBumperTriggered) {
    DebugPrintln("Left Bumper Triggered !", DBG_INFO, true);
    LeftBumperTriggered = false;
  }
  
  if (HorizontalTiltTriggered) {
    DebugPrintln("Horizontal Tilt sensor Triggered !", DBG_INFO, true);
    HorizontalTiltTriggered = false;
  }

  if (VerticalTiltTriggered) {
    DebugPrintln("Vertical Tilt sensor Triggered !", DBG_INFO, true);
    VerticalTiltTriggered = false;
  }

  BatteryChargeCurrentRead();
  MotorCurrentRead(MOTOR_CURRENT_RIGHT);

  int voltraw = analogRead(PIN_ESP_BAT_VOLT);     // TEST!
  int volt = map(voltraw, 0, 4095, 0, 17000);     // TEST!

  DebugPrint("Temp 1: " + String(TemperatureRead(TEMPERATURE_1_RED),1) + // " | Err1: " + String(Temp1ErrorCount) + 
             " | Temp 2: " + String(TemperatureRead(TEMPERATURE_2_BLUE),1) + //" | Err2: " + String(Temp2ErrorCount) + 
             " | Charge: " + String(BatteryChargeCurrent,2) + 
             " | MotorR: " + String(MotorCurrent[MOTOR_CURRENT_RIGHT],2) + 
             " | Volt: " + String(float(volt)/1000.0f,2), DBG_INFO, true);
             
//  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("T1: " + String(TemperatureRead(TEMPERATURE_1_RED),1) + " T2: " + String(TemperatureRead(TEMPERATURE_2_BLUE),1));

  lcd.setCursor(0,1);
  for (uint8_t i = 0; i < SONAR_COUNT; i++)
  {            // Loop through each sensor and display results.
    delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    unsigned int distance = sonar[i].ping_cm(SONAR_MAX_DISTANCE);
    DebugPrint(" | Sonar" + String(i+1) + ": " + String(distance));
    lcd.print("S" + String(i+1) + ":" + String(distance) + " ");
  }
  DebugPrintln("");

  lcd.setCursor(0,2);

  for (uint8_t i = 8; i < 12; i++){
    int key = IOExtend.digitalRead(i);
    if (!key) {DebugPrintln("Keypad key" + String(i-7) + " pressed", DBG_INFO, true);}
    lcd.print("K" + String(i-7) + ":" + String(key) + " ");
  }
  
  lcd.setCursor(0,3);
  lcd.print("B:" + String(BatteryChargeCurrent,1) + " ");
  lcd.print("R:" + String(MotorCurrent[MOTOR_CURRENT_RIGHT],0) + " ");
  lcd.print("L:" + String(MotorCurrent[MOTOR_CURRENT_LEFT],0) + " ");
//  lcd.print("C:" + String(MotorCurrent[MOTOR_CURRENT_CUT],0) + " ");

  MQTTclient.loop();

  SerialAndTelnet.handle();

  events();   // eztime refresh

  delay(500);
}