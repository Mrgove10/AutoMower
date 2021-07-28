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
    portENTER_CRITICAL_ISR(&g_TiltMux[TILT_HORIZONTAL]);
    g_TiltTriggered[TILT_HORIZONTAL] = (LastHorizontalTitltStatus == LOW);
    LastHorizontalTiltChange = millis();
    portEXIT_CRITICAL_ISR(&g_TiltMux[TILT_HORIZONTAL]);
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
    portENTER_CRITICAL_ISR(&g_TiltMux[TILT_VERTICAL]);
    g_TiltTriggered[TILT_VERTICAL] = (LastVerticalTiltStatus == LOW);
    LastVerticalTiltChange = millis();
    portEXIT_CRITICAL_ISR(&g_TiltMux[TILT_VERTICAL]);
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
  DebugPrintln("TiltSensorCheck start " + String(tilt), DBG_VERBOSE, true);

  if (tilt == TILT_HORIZONTAL)
  {
    DisplayClear();
  }
  DisplayPrint(0, 0, F("Tilt Tests"));

  int raw = digitalRead(g_tiltPin[tilt]);

  DebugPrintln(g_tiltStr[tilt] + " tilt input value: " + String(raw), DBG_VERBOSE, true);

  if (raw)
  {
    DebugPrintln(g_tiltStr[tilt] + " tilt sensor Ok", DBG_INFO, true);
    DisplayPrint(2, 2 + tilt, g_tiltStr[tilt] + " OK");
    delay(TEST_SEQ_STEP_WAIT);
    return true;
  }
  else
  {
    LogPrintln(g_tiltStr[tilt] + " tilt sensor is triggered", TAG_CHECK, DBG_ERROR);
    DisplayPrint(2, 2 + tilt, g_tiltStr[tilt] + " ACTIVE");
    delay(TEST_SEQ_STEP_WAIT + TEST_SEQ_STEP_ERROR_WAIT);
    return false;
  }
}

/**
 * Read, in a protected way, the status of the tilt sensor
 * @param Tilt int tilt number
 * @return boolean true if tilt activated, false if not
 */
bool TiltRead(int tilt)
{
  bool returnVal;
  portENTER_CRITICAL_ISR(&g_TiltMux[tilt]);
  returnVal = g_TiltTriggered[tilt];
  g_TiltTriggered[tilt] = false;
  portEXIT_CRITICAL_ISR(&g_TiltMux[tilt]);
  return returnVal;
}