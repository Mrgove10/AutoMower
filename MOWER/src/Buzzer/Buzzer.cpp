#include <Arduino.h>
#include "pin_definitions.h"
#include "myGlobals_definition.h"
#include "Environment_definitions.h"
#include "Utils/Utils.h"
#include "Display/Display.h"
#include "Buzzer/Buzzer.h"

/**
 * Buzzer Setup function
 *
 */
void BuzzerSetup(void)
{
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RESOLUTION);
  ledcAttachPin(PIN_ESP_BUZZER, BUZZER_CHANNEL);
  DebugPrintln("Buzzer setup Done", DBG_VERBOSE, true);
}

/**
 * Buzzer check function
 *
 */
void BuzzerCheck(void)
{
  playTune(g_startTune, sizeof(g_startTune) / sizeof(noteStruct));
}


/**
 * Buzzer tune play function
 * 
 * @param tune pointer to array of structures containing tune to play
 * @param length of array
 * @param repeats number of times tune is played
 * 
 */
void playTune(noteStruct * tune, const int length, const int repeats)
{
  for (int c = 0; c < repeats; c++)
  {
    for (int note = 0; note < length; note++)
    {
      ledcWriteTone(BUZZER_CHANNEL, tune[note].tone);
      ledcWrite(BUZZER_CHANNEL, tune[note].frequency);
      delay(tune[note].duration);
      ledcWrite(BUZZER_CHANNEL, 0);
      delay(tune[note].pause);
    }
    ledcWrite(BUZZER_CHANNEL, 0);
    delay(500);
  }
}
