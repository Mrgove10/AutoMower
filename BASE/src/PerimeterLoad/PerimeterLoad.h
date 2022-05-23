#ifndef perimeterload_h
#define perimeterload_h

/**
 * I2C INA219 Perimeter load Current Sensor Setup function
 *
 */
void PerimeterLoadCurrentSensorSetup();

/**
 * Checks to see if perimeter INA219 I2C current sensor is connected (and hopefully functioning)
 * 
 * @return true if INA219 current sensor is ok
 */
bool PerimeterLoadCurrentSensorCheck(void);

/**
 * Function to test and read perimeter load current
 * 
 * * @param Now true to force an immediate read
 * * @param Reset true to force an average value reset
 * * @return true if charge current could be read
 */
bool PerimeterLoadCurrentRead(const bool Now = false, const bool Reset = false);

#endif