#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "Keypad/Keypad.h"
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
  IOExtend.pullUp(PIN_MCP_KEYPAD_1, HIGH);  // turn on a 100K pullup internally
  IOExtend.pullUp(PIN_MCP_KEYPAD_2, HIGH);  // turn on a 100K pullup internally
  IOExtend.pullUp(PIN_MCP_KEYPAD_3, HIGH);  // turn on a 100K pullup internally
  IOExtend.pullUp(PIN_MCP_KEYPAD_4, HIGH);  // turn on a 100K pullup internally
  
  DebugPrintln("Keypad setup Done", DBG_VERBOSE, true);

}