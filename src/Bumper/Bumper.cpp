#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Bumper/Bumper.h"
#include "Utils/Utils.h"

/**
 * Left Bumper ISR function
 * 
 */
ICACHE_RAM_ATTR void LeftBumperISR(void)
{
  static unsigned long LastLeftBumperChange = 0;
  static bool LastLeftBumperStatus = false;     // assumption is that bumper is functionning on start and in not "bumped" (normally closed contact)

  LastLeftBumperStatus = !LastLeftBumperStatus;  // Capture every status change

  if (millis() - LastLeftBumperChange > BUMPER_DEBOUNCE_TIMEOUT) {
    LeftBumpertriggered = (LastLeftBumperStatus == HIGH);
    LastLeftBumperChange = millis();
  }
}

/**
 * Right Bumper ISR function
 * 
 */
ICACHE_RAM_ATTR void RightBumperISR(void)
{
  static unsigned long LastRightBumperChange = 0;
  static bool LastRightBumperStatus = false;     // assumption is that bumper is functionning on start and in not "bumped" (normally closed contact)

  LastRightBumperStatus = !LastRightBumperStatus;  // Capture every status change

  if (millis() - LastRightBumperChange > BUMPER_DEBOUNCE_TIMEOUT) {
    RightBumpertriggered = (LastRightBumperStatus == HIGH);
    LastRightBumperChange = millis();
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
 * Checks to see if bumper sensor is connected (and hopefully functionning)
 * @param bumper int    bumper number
 * @return boolean true if rain sensor check is ok
 */
bool BumperSensorCheck(int bumper)
{
  DebugPrintln("BumperSensorCheck start " + String(bumper), DBG_VERBOSE, true);

  int bumperPin = 0;
  String bumperStr = "";

  if (bumper == BUMPER_LEFT) 
  {
    bumperPin = PIN_ESP_BUMPER_LEFT;
    bumperStr = "Left";
  }
  if (bumper == BUMPER_RIGHT) {
    bumperPin = PIN_ESP_BUMPER_RIGHT;
    bumperStr = "Right";
  }

  int raw = digitalRead(bumperPin);
  
  DebugPrintln(bumperStr + " bumper input value: " + String(raw), DBG_VERBOSE, true);
  
  if (!raw) {
    DebugPrintln(bumperStr + " bumper Ok", DBG_INFO, true);
    return true;
  }
  else
  {
    LogPrintln(bumperStr + " bumper not found or triggered", TAG_CHECK, DBG_ERROR);
//    DebugPrintln(bumperStr + " bumper not found or triggered", DBG_ERROR, true);
    return false;
  }
}
