#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Keypad/Keypad.h"
#include "IOExtender/IOExtender.h"
#include "Utils/Utils.h"

/**
 * Keypad Setup function
 * 
 */
void KeypadSetup(void)
{
  IOExtend.pinMode(PIN_MCP_KEYPAD_1, INPUT);
  IOExtend.pinMode(PIN_MCP_KEYPAD_2, INPUT);
  IOExtend.pinMode(PIN_MCP_KEYPAD_3, INPUT);
  IOExtend.pinMode(PIN_MCP_KEYPAD_4, INPUT);
  IOExtend.pullUp(PIN_MCP_KEYPAD_1, HIGH); // turn on a 100K pullup internally
  IOExtend.pullUp(PIN_MCP_KEYPAD_2, HIGH); // turn on a 100K pullup internally
  IOExtend.pullUp(PIN_MCP_KEYPAD_3, HIGH); // turn on a 100K pullup internally
  IOExtend.pullUp(PIN_MCP_KEYPAD_4, HIGH); // turn on a 100K pullup internally

  DebugPrintln("Keypad setup Done", DBG_VERBOSE, true);
}

/**
 * Keypad Read function
 * 
 * Keypad status is memorised in global variables
 */
void KeypadRead(void)
{
  static unsigned long LastKeypadRead = 0;
  uint8_t IOregister;
  const uint8_t KeyMasks[KEYPAD_MAX_KEYS] = {0X2, 0X1, 0X8, 0x4};

  if ((millis() - LastKeypadRead > KEYPAD_READ_INTERVAL))
  {
    IOregister = IOExtendProtectedGPIORead(KEYPAD_GPIO) & 0XF; // logical AND to remove heigher weight bits
    for (uint8_t key = 0; key < KEYPAD_MAX_KEYS; key++)
    {
      g_KeyPressed[key] = ((IOregister & KeyMasks[key]) ^ KeyMasks[key]) == KeyMasks[key]; // logical AND to isolate bit of interest
    }

    LastKeypadRead = millis();
  }
}
