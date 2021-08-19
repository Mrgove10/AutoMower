#ifndef ioextend_h
#define ioextend_h

/**
 * I2C MCP23017 IO Extenter Setup function
 * 
 */
void IOExtendSetup(void);

/**
 * I2C MCP23017 IO Extenter protected digital write function
 * @param pin to write
 * @param value to write 
 */

void IOExtendProtectedWrite(const uint8_t pin, const uint8_t value);

/**
 * I2C MCP23017 IO Extenter protected digital read function
 * @param pin to read
 * @return read value
 */
uint8_t IOExtendProtectedRead(const uint8_t pin);

/**
 * I2C MCP23017 IO Extenter protected digital GIPO read function
 * @param gpio to read
 * @return read value
 */
uint8_t IOExtendProtectedGPIORead(const uint8_t gpio);

#endif