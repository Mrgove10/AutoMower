#ifndef Compass_h
#define Compass_h

/**
 * I2C HMC5883L Compasss Sensor Setup function
 *
 */
void CompassSensorSetup();

/**
 * Function to test and read I2C HMC5883L Compasss Sensor
 *  
 * @param Now true to force an immediate read
 */
void CompassRead(const bool Now = false);

/**
 * Checks to see if I2C HMC5883L Compasss Sensor is connected (and hopefully functionning)
 * @return true if current sensor is ok
 */
bool CompassSensorCheck(void);

/**
 * Displays I2C HMC5883L Compasss Sensor details
 */
void DisplayCompassDetails(void);

#endif