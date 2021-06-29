
#include <Arduino.h>
#include "myGlobals_definition.h"
#include "mySetup.h"
#include "Utils/Utils.h"
#include "Temperature/Temperature.h"
#include "EEPROM/EEPROM.h"
#include <pin_definitions.h>

#include <NewPing.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>

// multiplexer
Adafruit_MCP23017 mcp;

// Ultrasonic sensors
/*
NewPing sonar[SONAR_NUM] = {                                           // Sensor object array.
    NewPing(PIN_ESP_SONAR_CENTER, PIN_ESP_SONAR_CENTER, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
    NewPing(PIN_ESP_SONAR_LEFT, PIN_ESP_SONAR_LEFT, MAX_DISTANCE),
    NewPing(PIN_ESP_SONAR_RIGHT, PIN_ESP_SONAR_RIGHT, MAX_DISTANCE)};
*/

/*
// Temperature sensors
OneWire ds(PIN_ESP_TEMP);
*/

/*
// screen
LiquidCrystal_I2C lcd(0x38);
*/

/*
void multiplexSetup()
{
  mcp.begin();
  mcp.pinMode(0, INPUT);
  mcp.pullUp(0, HIGH); // turn on a 100K pullup internally
}
*/

/*
void lcdSetup()
{
  lcd.begin(SCREEN_COL, SCREEN_LINES); // columns, lines
  lcd.clear();
}
*/

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

  DebugPrintln("Temp 1: " + String(temperatureRead(TEMPERATURE_1_RED),1) + " | Err1: " + String(Temp1ErrorCount) + " | Temp 2: " + String(temperatureRead(TEMPERATURE_2_BLUE),1) + " | Err2: " + String(Temp2ErrorCount), DBG_INFO, true);
//  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("T1: " + String(temperatureRead(TEMPERATURE_1_RED),1) + " | T2: " + String(temperatureRead(TEMPERATURE_2_BLUE),1));

  MQTTclient.loop();

  SerialAndTelnet.handle();

  events();   // eztime refresh

  delay(500);

  /*
  for (uint8_t i = 0; i < SONAR_NUM; i++)
  {            // Loop through each sensor and display results.
    delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    Serial.print(i);
    Serial.print("=");
    Serial.print(sonar[i].ping_cm());
    Serial.print("cm ");
  }
  */
}