#include <Arduino.h>
#include "pin_definitions.h"
#include "Environment_definitions.h"
#include "myGlobals_definition.h"
#include "IOExtender/IOExtender.h"
#include "Utils/Utils.h"

/**
 * I2C MCP23017 IO Extenter Setup function
 * 
 */
void IOExtendSetup(void)
{
  IOExtend.begin();       // default address 0X20
  
  DebugPrintln("MCP I2C IO Extender setup Done", DBG_VERBOSE, true);
}