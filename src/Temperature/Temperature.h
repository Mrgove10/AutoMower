#ifndef temperature_h
#define temperature_h

#include "pin_definitions.h"
#define TEMPERATURE_PRECISION 9

#define TEMPERATURE_1_RED 1
#define TEMPERATURE_2_BLUE 2

/**
 * Temperature sensor setup function
 * 
 */
void TemperatureSensorSetup(void);

/**
 * Checks to see if Temperature sensor is connected and functionning
 * @param sensor int to check
 * @return true if Temperature sensor check is ok
 */
bool TemperatureSensorCheck(int sensor);

/**
 * Function to read temperature
 * @param device int functional sensor to read temperature from
 * @return float sensor temperature
 */
float TemperatureRead(int sensor);

/**
 * Temperature sensor device address formating function
 * @param device DeviceAddress to format
 * @return String displaying a formated device address
 */
String TempSensorAddress(DeviceAddress device);

#endif