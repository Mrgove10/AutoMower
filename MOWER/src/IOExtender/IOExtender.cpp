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
  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  IOExtend.begin(); // default address 0X20

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  DebugPrintln("MCP I2C IO Extender setup Done", DBG_VERBOSE, true);
}

/**
 * I2C MCP23017 IO Extenter protected digital write function
 * @param pin to write
 * @param value to write 
 */
void IOExtendProtectedWrite(const uint8_t pin, const uint8_t value)
{
  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  IOExtend.digitalWrite(pin, value);

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);
}

/**
 * I2C MCP23017 IO Extenter protected digital read function
 * @param pin to write
 * @return read value
 */
uint8_t IOExtendProtectedRead(const uint8_t pin)
{
  uint8_t value;

  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  value = IOExtend.digitalRead(pin);

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  return value;
}

/**
 * I2C MCP23017 IO Extenter protected digital GIPO read function
 * @param gpio to read
 * @return read value
 */
uint8_t IOExtendProtectedGPIORead(const uint8_t gpio)

{
  uint8_t value;

  // Ensure exlusive access to I2C
  xSemaphoreTake(g_I2CSemaphore, portMAX_DELAY);

  value = IOExtend.readGPIO(gpio);

  // Free access to I2C for other tasks
  xSemaphoreGive(g_I2CSemaphore);

  return value;
}


