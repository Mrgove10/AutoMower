#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Fan/Fan.h"
#include "Temperature/Temperature.h"
#include "IOExtender/IOExtender.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

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

  if (Fan == 0)
  {
    DisplayClear();
    DisplayPrint(0, 0, F("Fan Test"));
  }
  DisplayPrint(2, 2 + Fan, "Fan " + String(Fan + 1) + " Started");

  FanStart(Fan);

  delay(FAN_TEST_DURATION);

  FanStop(Fan);
  DisplayPrint(2, 2 + Fan, "Fan " + String(Fan + 1) + " Stopped", true);

  delay(TEST_SEQ_STEP_WAIT);

  DebugPrintln("Fan " + String(Fan + 1) + " test completed", DBG_INFO, true);
}

/**
 * Fan Start function
 * @param Fan to start
 */
void FanStart(const int Fan)
{
  IOExtendProtectedWrite(g_FanPin[Fan], HIGH);
  g_FanOn[Fan] = true;
}

/**
 * Fan Stop function
 * @param Fan to stop
 */
void FanStop(const int Fan)
{
  IOExtendProtectedWrite(g_FanPin[Fan], LOW);
  g_FanOn[Fan] = false;
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
    float temperature = TemperatureRead(Fan, true);
    if (!g_FanOn[Fan] && (temperature != UNKNOWN_FLOAT) && (temperature > FAN_START_THRESHOLD))
    {
      DebugPrintln("Fan " + String(Fan + 1) + " Started", DBG_INFO, true);
      g_FanOn[Fan] = true;
      FanStart(Fan);
    }
    if (g_FanOn[Fan] && (temperature != UNKNOWN_FLOAT) && (temperature < FAN_STOP_THRESHOLD))
    {
      DebugPrintln("Fan " + String(Fan + 1) + " Stopped", DBG_INFO, true);
      g_FanOn[Fan] = false;
      FanStop(Fan);
    }
    LastFanCheck[Fan] = millis();
  }
}
