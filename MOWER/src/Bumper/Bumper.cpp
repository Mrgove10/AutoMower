#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Bumper/Bumper.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Left Bumper ISR function
 * 
 */
ICACHE_RAM_ATTR void LeftBumperISR(void)
{
  static unsigned long LastLeftBumperChange = 0;
  static bool LastLeftBumperStatus = false; // assumption is that bumper is functioning on start and in not "bumped" (normally closed contact)

  LastLeftBumperStatus = !LastLeftBumperStatus; // Capture every status change

  if (millis() - LastLeftBumperChange > BUMPER_DEBOUNCE_TIMEOUT)
  {
    portENTER_CRITICAL_ISR(&g_BumperMux[BUMPER_LEFT]);
    g_BumperTriggered[BUMPER_LEFT] = (LastLeftBumperStatus == HIGH);
    LastLeftBumperChange = millis();
    portEXIT_CRITICAL_ISR(&g_BumperMux[BUMPER_LEFT]);
  }
}

/**
 * Right Bumper ISR function
 * 
 */
ICACHE_RAM_ATTR void RightBumperISR(void)
{
  static unsigned long LastRightBumperChange = 0;
  static bool LastRightBumperStatus = false; // assumption is that bumper is functioning on start and in not "bumped" (normally closed contact)

  LastRightBumperStatus = !LastRightBumperStatus; // Capture every status change

  if (millis() - LastRightBumperChange > BUMPER_DEBOUNCE_TIMEOUT)
  {
    portENTER_CRITICAL_ISR(&g_BumperMux[BUMPER_RIGHT]);
    g_BumperTriggered[BUMPER_RIGHT] = (LastRightBumperStatus == HIGH);
    LastRightBumperChange = millis();
    portEXIT_CRITICAL_ISR(&g_BumperMux[BUMPER_RIGHT]);
  }
}

/**
 * Bumper Setup function
 * 
 */
void BumperSetup(void)
{
  pinMode(PIN_ESP_BUMPER_LEFT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ESP_BUMPER_LEFT), LeftBumperISR, CHANGE);

  pinMode(PIN_ESP_BUMPER_RIGHT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ESP_BUMPER_RIGHT), RightBumperISR, CHANGE);

  DebugPrintln("Bumper setup Done", DBG_VERBOSE, true);
}

/**
 * Checks to see if bumper sensor is connected (and hopefully functioning)
 * @param bumper int    bumper number
 * @return boolean true if rain sensor check is ok
 */
bool BumperSensorCheck(int bumper)
{
  DebugPrintln("BumperSensorCheck start " + String(bumper), DBG_VERBOSE, true);
  if (bumper == BUMPER_LEFT)
  {
    DisplayClear();
  }
  DisplayPrint(0, 0, F("Bumper Tests"));

  int raw = digitalRead(g_bumperPin[bumper]);

  DebugPrintln(g_bumperStr[bumper] + " bumper input value: " + String(raw), DBG_VERBOSE, true);

  if (!raw)
  {
    DebugPrintln(g_bumperStr[bumper] + " bumper Ok", DBG_INFO, true);
    DisplayPrint(2, 2 + bumper, g_bumperStr[bumper] + " OK");
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(g_bumperStr[bumper] + " bumper not found or is triggered", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 2 + bumper, g_bumperStr[bumper] + " ERROR");
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Read, in a protected way, the status of a bumper sensor
 * @param bumper int bumper number
 * @return boolean true if bumper activated, false if not
 */
bool BumperRead(int bumper)
{
  bool returnVal;
  portENTER_CRITICAL_ISR(&g_BumperMux[bumper]);
  returnVal = g_BumperTriggered[bumper];
  g_BumperTriggered[bumper] = false;
  portEXIT_CRITICAL_ISR(&g_BumperMux[bumper]);
  return returnVal;
}
