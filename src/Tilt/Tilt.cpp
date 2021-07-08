#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Tilt/Tilt.h"
#include "Utils/Utils.h"
#include "Display/Display.h"

/**
 * Horizontal Tilt sensor ISR function
 * 
 */
ICACHE_RAM_ATTR void horizontalTiltISR(void)
{
  static unsigned long LastHorizontalTiltChange = 0;
  static bool LastHorizontalTitltStatus = false; // assumption is that tilt is functionning on start and in not triggered (normally open contact)

  LastHorizontalTitltStatus = !LastHorizontalTitltStatus; // Capture every status change

  if (millis() - LastHorizontalTiltChange > TILT_DEBOUNCE_TIMEOUT)
  {
    HorizontalTiltTriggered = (LastHorizontalTitltStatus == LOW);
    LastHorizontalTiltChange = millis();
  }
}

/**
 * Vertical Tilt sensor ISR function
 * 
 */
ICACHE_RAM_ATTR void verticalTiltISR(void)
{
  static unsigned long LastVerticalTiltChange = 0;
  static bool LastVerticalTiltStatus = false; // assumption is that tilt is functionning on start and in not triggered (normally open contact)

  LastVerticalTiltStatus = !LastVerticalTiltStatus; // Capture every status change

  if (millis() - LastVerticalTiltChange > TILT_DEBOUNCE_TIMEOUT)
  {
    VerticalTiltTriggered = (LastVerticalTiltStatus == LOW);
    LastVerticalTiltChange = millis();
  }
}

/**
 * Tilt sensor Setup function
 * 
 */
void TiltSetup(void)
{
  pinMode(PIN_ESP_TILT_HORIZONTAL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ESP_TILT_HORIZONTAL), horizontalTiltISR, CHANGE);

  pinMode(PIN_ESP_TILT_VERTICAL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ESP_TILT_VERTICAL), verticalTiltISR, CHANGE);

  DebugPrintln("Tilt setup Done", DBG_VERBOSE, true);
}

/**
 * Checks to see if tilt sensor is activated on mower startup
 * @param Tilt int    bumper number
 * @return boolean true if sensor check is ok
 */
bool TiltSensorCheck(int tilt)
{
  int tiltPin = 0;
  String tiltStr = "";

  DebugPrintln("TiltSensorCheck start " + String(tilt), DBG_VERBOSE, true);

  if (tilt == 1)
  {
    DisplayClear();
  }
  DisplayPrint(0, 0, F("Tilt Tests"));

  if (tilt == TILT_HORIZONTAL)
  {
    tiltPin = PIN_ESP_TILT_HORIZONTAL;
    tiltStr = "Horizontal ";
  }

  if (tilt == TILT_VERTICAL)
  {
    tiltPin = PIN_ESP_TILT_VERTICAL;
    tiltStr = "Vertical";
  }

  int raw = digitalRead(tiltPin);

  DebugPrintln(tiltStr + " tilt input value: " + String(raw), DBG_VERBOSE, true);

  if (raw)
  {
    DebugPrintln(tiltStr + " tilt sensor Ok", DBG_INFO, true);
    DisplayPrint(2, 1 + tilt, tiltStr + " OK");
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(tiltStr + " tilt sensor is triggered", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 1 + tilt, tiltStr + " ACTIVE");
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}
