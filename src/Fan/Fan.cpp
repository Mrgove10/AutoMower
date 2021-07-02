#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Fan/Fan.h"
#include "Temperature/Temperature.h"
#include "Utils/Utils.h"

/**
 * Fan Setup function
 */
void FanSetup()
{
  IOExtend.pinMode(PIN_MCP_FAN_1, OUTPUT);
  IOExtend.pinMode(PIN_MCP_FAN_2, OUTPUT);

  FanStop(FAN_1_RED);
  FanStop(FAN_2_BLUE);
  
  DebugPrintln("Fan setup Done", DBG_VERBOSE, true);
}

/**
 * Fan test function
 * @param Fan to test
 */
void FanTest(const int Fan)
{
  DebugPrintln("Fan " + String(Fan + 1) + " test started", DBG_INFO, true);

  if (Fan == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Fan Test"));
  }
  lcd.setCursor(2, 2 + Fan);
  lcd.print (F("Fan "));
  lcd.print (Fan + 1);

  FanStart(Fan);

  delay(FAN_TEST_DURATION);

  FanStop(Fan);

  lcd.print(F(" completed"));

  DebugPrintln("Fan " + String(Fan + 1) + " test completed", DBG_INFO, true);
}

/**
 * Fan Start function
 * @param Fan to start
 */
void FanStart(const int Fan)
{
  IOExtend.digitalWrite(FanPin[Fan], HIGH);
  FanOn[Fan] = true;
}

/**
 * Fan Stop function
 * @param Fan to stop
 */
void FanStop(const int Fan)
{
  IOExtend.digitalWrite(FanPin[Fan], LOW);

  FanOn[Fan] = false;
}

/**
 * Fan Check function
 * @param Fan to check
 * @param Now true to force immediate check
 */
void FanCheck(const int Fan, const bool Now)
{
  static unsigned long LastFanCheck[FAN_COUNT] = {0, 0};

  if ((millis() - LastFanCheck[Fan] > FAN_UPDATE_INTERVAL) || Now) 
  {
    float temperature = TemperatureRead(Fan,true);
    if ( !FanOn[Fan] && (temperature != UNKNOWN_FLOAT) && (temperature > FAN_START_THRESHOLD))
    {
      DebugPrintln("Fan " + String(Fan + 1) + " Started", DBG_INFO, true);
      FanOn[Fan] = true;
      FanStart(Fan);
    }
    if ( FanOn[Fan] && (temperature != UNKNOWN_FLOAT) && (temperature < FAN_STOP_THRESHOLD))
    {
      DebugPrintln("Fan " + String(Fan + 1) + " Stopped", DBG_INFO, true);
      FanOn[Fan] = false;
      FanStop(Fan);
    }
    LastFanCheck[Fan] = millis();
  }
}
