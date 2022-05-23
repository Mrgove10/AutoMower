#ifndef temperature_h
#define temperature_h

#include "pin_definitions.h"

/**
 * Temperature sensor setup function
 * 
 */
void TemperatureSensorSetup(void);

/**
 * Checks to see if Temperature sensor is connected and functioning
 * @param sensor int to check
 * @return true if Temperature sensor check is ok
 */
bool TemperatureSensorCheck(int sensor);

/**
 * Function to read temperature
 * @param device int functional sensor to read temperature from
 * @param Now optional bool to force immediate temperature read
 * 
 * @return float sensor temperature
 */
float TemperatureRead(int sensor, const bool Now = false);

/**
 * Temperature sensor device address formatting function
 * @param device DeviceAddress to format
 * @return String displaying a formatted device address
 */
String TempSensorAddress(DeviceAddress device);

#endif